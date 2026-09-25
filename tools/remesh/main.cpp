/*-----------------------------------------------------------------------------------------------------------------------|
| MIT License (MIT)                                                                                                      |
| Copyright (C) 2026 by Michel Braz de Morais <michel.braz.morais@gmail.com>                                             |
|                                                                                                                        |
| Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated           |
| documentation files (the "Software"), to deal in the Software without restriction, including without limitation       |
| the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and       |
| to permit persons to whom the Software is furnished to do so, subject to the following conditions:                     |
|                                                                                                                        |
| The above copyright notice and this permission notice shall be included in all copies or substantial portions.         |
| THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   |
| WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR  |
| COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR       |
| OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.       |
|-----------------------------------------------------------------------------------------------------------------------*/

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/detect_features.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../planar/uv-transfer.h"

namespace PMP = CGAL::Polygon_mesh_processing;
using KERNEL = CGAL::Exact_predicates_inexact_constructions_kernel;
using MESH = CGAL::Surface_mesh<KERNEL::Point_3>;
using FACE = MESH::Face_index;
using EDGE = MESH::Edge_index;
using VERTEX = MESH::Vertex_index;
using PRIMITIVE = CGAL::AABB_face_graph_triangle_primitive<MESH>;
using TRAITS = CGAL::AABB_traits_3<KERNEL, PRIMITIVE>;
using TREE = CGAL::AABB_tree<TRAITS>;

static bool sameUv(const mbm_cgal_uv::UV &a, const mbm_cgal_uv::UV &b)
{
    return a[0] == b[0] && a[1] == b[1];
}

static mbm_cgal_uv::UV uvAt(const mbm_cgal_uv::FACE &face, VERTEX vertex)
{
    return mbm_cgal_uv::at(face, vertex);
}

static std::size_t labelCharts(MESH &mesh, const mbm_cgal_uv::INPUT &input,
                               MESH::Property_map<FACE, std::size_t> &chartMap,
                               MESH::Property_map<EDGE, bool> &constrained,
                               double featureAngle)
{
    const auto unset = std::size_t(-1);
    auto sharp = mesh.add_property_map<EDGE, bool>("e:mbm_sharp", false).first;
    PMP::detect_sharp_edges(mesh, featureAngle, sharp);
    for (FACE face : mesh.faces()) put(chartMap, face, unset);
    for (EDGE edge : mesh.edges())
    {
        const auto h = mesh.halfedge(edge), opposite = mesh.opposite(h);
        const FACE left = mesh.face(h), right = mesh.face(opposite);
        if (left == MESH::null_face() || right == MESH::null_face()) continue;
        const auto &a = input.faces.at(left.idx());
        const auto &b = input.faces.at(right.idx());
        const VERTEX source = mesh.source(h), target = mesh.target(h);
        const bool material = a.material != b.material;
        const bool seam = !sameUv(uvAt(a, source), uvAt(b, source)) ||
                          !sameUv(uvAt(a, target), uvAt(b, target));
        put(constrained, edge, material || seam || get(sharp, edge));
    }

    std::size_t count = 0;
    for (FACE seed : mesh.faces())
    {
        if (get(chartMap, seed) != unset) continue;
        const std::size_t id = count++;
        std::vector<FACE> pending{seed};
        put(chartMap, seed, id);
        for (std::size_t cursor = 0; cursor < pending.size(); ++cursor)
        {
            const FACE face = pending[cursor];
            const auto start = mesh.halfedge(face);
            auto h = start;
            do
            {
                const auto opposite = mesh.opposite(h);
                const FACE next = mesh.face(opposite);
                if (next != MESH::null_face() && get(chartMap, next) == unset &&
                    !get(constrained, mesh.edge(h)))
                {
                    put(chartMap, next, id);
                    pending.push_back(next);
                }
                h = mesh.next(h);
            } while (h != start);
        }
    }
    return count;
}

static double diagonal(const MESH &mesh)
{
    CGAL::Bbox_3 bounds;
    bool first = true;
    for (VERTEX vertex : mesh.vertices())
    {
        const auto box = mesh.point(vertex).bbox();
        if (first) { bounds = box; first = false; }
        else bounds = bounds + box;
    }
    const double x = bounds.xmax() - bounds.xmin();
    const double y = bounds.ymax() - bounds.ymin();
    const double z = bounds.zmax() - bounds.zmin();
    return std::sqrt(x*x + y*y + z*z);
}

static std::size_t components(MESH &mesh)
{
    auto map = mesh.add_property_map<FACE, std::size_t>("f:component", 0).first;
    return PMP::connected_components(mesh, map);
}

static double sampledDistance(const MESH &source, const MESH &target)
{
    TREE tree(faces(target).first, faces(target).second, target);
    tree.accelerate_distance_queries();
    double maximum = 0;
    for (FACE face : source.faces())
    {
        const auto h = source.halfedge(face);
        const auto &a = source.point(source.source(h));
        const auto &b = source.point(source.target(h));
        const auto &c = source.point(source.target(source.next(h)));
        for (const auto &point : {a, b, c, CGAL::midpoint(a,b), CGAL::midpoint(b,c),
                                  CGAL::midpoint(c,a), CGAL::centroid(a,b,c)})
            maximum = std::max(maximum, CGAL::to_double(tree.squared_distance(point)));
    }
    return std::sqrt(maximum);
}

static std::array<double, 3> barycentric(const KERNEL::Point_3 &point,
                                         const KERNEL::Point_3 &a,
                                         const KERNEL::Point_3 &b,
                                         const KERNEL::Point_3 &c)
{
    const auto v0 = b - a, v1 = c - a, v2 = point - a;
    const double d00 = CGAL::to_double(v0 * v0), d01 = CGAL::to_double(v0 * v1);
    const double d11 = CGAL::to_double(v1 * v1), d20 = CGAL::to_double(v2 * v0);
    const double d21 = CGAL::to_double(v2 * v1);
    const double denominator = d00 * d11 - d01 * d01;
    if (denominator <= 0 || !std::isfinite(denominator))
        throw std::runtime_error("degenerate source triangle during UV transfer");
    const double v = (d11 * d20 - d01 * d21) / denominator;
    const double w = (d00 * d21 - d01 * d20) / denominator;
    return {1.0 - v - w, v, w};
}

static mbm_cgal_uv::UV transferUv(const KERNEL::Point_3 &point, std::size_t chart,
                                 const std::vector<std::unique_ptr<TREE>> &trees,
                                 const MESH &source, const mbm_cgal_uv::INPUT &input)
{
    const auto [closest, face] = trees.at(chart)->closest_point_and_primitive(point);
    const auto &sourceFace = input.faces.at(face.idx());
    const auto weights = barycentric(closest, source.point(sourceFace.vertices[0]),
                                     source.point(sourceFace.vertices[1]),
                                     source.point(sourceFace.vertices[2]));
    mbm_cgal_uv::UV uv{};
    for (unsigned i = 0; i < 3; ++i)
        for (unsigned axis = 0; axis < 2; ++axis)
            uv[axis] += weights[i] * sourceFace.uv[i][axis];
    if (!std::isfinite(uv[0]) || !std::isfinite(uv[1]))
        throw std::runtime_error("non-finite transferred UV");
    return uv;
}

static void writeObj(const std::string &path, const MESH &mesh,
                     const mbm_cgal_uv::INPUT &input,
                     const MESH::Property_map<FACE, std::size_t> &chartMap,
                     const std::vector<std::string> &materials,
                     const std::vector<std::unique_ptr<TREE>> &trees,
                     const MESH &source)
{
    std::ostringstream output;
    output << std::setprecision(17) << "# MBM CGAL isotropic remesh\n";
    for (const auto &library : input.libraries) output << "mtllib " << library << '\n';
    std::map<VERTEX, std::size_t> indices;
    for (VERTEX vertex : mesh.vertices())
    {
        indices[vertex] = indices.size() + 1;
        const auto &point = mesh.point(vertex);
        output << "v " << point.x() << ' ' << point.y() << ' ' << point.z() << '\n';
    }
    std::size_t uvIndex = 1;
    for (FACE face : mesh.faces())
    {
        const auto chart = get(chartMap, face);
        output << "usemtl " << materials.at(chart) << '\n';
        std::array<std::size_t, 3> vertexIds{};
        std::array<std::size_t, 3> textureIds{};
        auto h = mesh.halfedge(face);
        const auto start = h;
        for (unsigned corner = 0; corner < 3; ++corner)
        {
            const VERTEX vertex = mesh.source(h);
            const auto uv = transferUv(mesh.point(vertex), chart, trees, source, input);
            vertexIds[corner] = indices.at(vertex);
            textureIds[corner] = uvIndex++;
            output << "vt " << uv[0] << ' ' << uv[1] << '\n';
            h = mesh.next(h);
        }
        if (h != start) throw std::runtime_error("remeshed face is not triangular");
        output << 'f';
        for (unsigned corner = 0; corner < 3; ++corner)
            output << ' ' << vertexIds[corner] << '/' << textureIds[corner];
        output << '\n';
    }
    const auto temporary = std::filesystem::path(path).string() + ".tmp";
    std::ofstream file(temporary, std::ios::binary);
    if (!file || !(file << output.str())) throw std::runtime_error("cannot write OBJ output");
    file.close();
    std::filesystem::rename(temporary, path);
}

int main(int argc, char **argv)
{
    if (argc < 4 || argc > 7)
    {
        std::cerr << "Usage: mbm-cgal-remesh input.obj output.obj edge-length-fraction [iterations [feature-angle-deg [report-path]]]\n";
        return 2;
    }
    std::ofstream reportFile;
    try
    {
        const auto number = [](const char *value) {
            std::size_t consumed = 0;
            const double result = std::stod(value, &consumed);
            if (consumed != std::string(value).size()) throw std::runtime_error("invalid numeric argument");
            return result;
        };
        if (std::filesystem::exists(argv[2])) throw std::runtime_error("output already exists; choose a new file");
        if (argc == 7)
        {
            if (std::filesystem::exists(argv[6])) throw std::runtime_error("report already exists");
            reportFile.open(argv[6]);
            if (!reportFile) throw std::runtime_error("cannot open report");
        }
        if (std::filesystem::path(argv[1]).extension() != ".obj" ||
            std::filesystem::path(argv[2]).extension() != ".obj")
            throw std::runtime_error("input and output must be .obj files");
        const double fraction = number(argv[3]);
        const double rawIterations = argc >= 5 ? number(argv[4]) : 3;
        if (!std::isfinite(rawIterations) || rawIterations < 1 || rawIterations > 10 ||
            std::floor(rawIterations) != rawIterations)
            throw std::runtime_error("iterations range: 1..10");
        const unsigned iterations = static_cast<unsigned>(rawIterations);
        const double featureAngle = argc >= 6 ? number(argv[5]) : 45.0;
        if (!std::isfinite(fraction) || fraction <= 0 || fraction > 0.25)
            throw std::runtime_error("edge-length fraction range: (0..0.25]");
        if (!std::isfinite(featureAngle) || featureAngle < 0 || featureAngle > 180)
            throw std::runtime_error("feature angle range: 0..180");

        MESH source;
        const auto input = mbm_cgal_uv::read(argv[1], source);
        if (source.number_of_faces() == 0 || !CGAL::is_triangle_mesh(source) || !CGAL::is_valid_polygon_mesh(source))
            throw std::runtime_error("input is not a valid manifold triangle mesh; no automatic repair performed");
        for (const auto &face : input.faces)
            if (CGAL::collinear(source.point(face.vertices[0]), source.point(face.vertices[1]),
                                source.point(face.vertices[2])))
                throw std::runtime_error("input contains a degenerate triangle; no automatic repair performed");
        const double boundsDiagonal = diagonal(source);
        if (!std::isfinite(boundsDiagonal) || boundsDiagonal <= 0)
            throw std::runtime_error("invalid input bounds");
        const double targetEdgeLength = fraction * boundsDiagonal;
        const auto begin = std::chrono::steady_clock::now();

        auto chartMap = source.add_property_map<FACE, std::size_t>("f:mbm_chart", std::size_t(-1)).first;
        auto constraints = source.add_property_map<EDGE, bool>("e:mbm_constraint", false).first;
        const auto chartCount = labelCharts(source, input, chartMap, constraints, featureAngle);

        MESH result = source;
        const auto resultChartsProperty = result.property_map<FACE, std::size_t>("f:mbm_chart");
        const auto resultConstraintsProperty = result.property_map<EDGE, bool>("e:mbm_constraint");
        if (!resultChartsProperty || !resultConstraintsProperty)
            throw std::runtime_error("remeshing property maps are missing");
        const auto resultCharts = *resultChartsProperty;
        const auto resultConstraints = *resultConstraintsProperty;
        PMP::isotropic_remeshing(faces(result), targetEdgeLength, result,
            CGAL::parameters::number_of_iterations(iterations)
                .edge_is_constrained_map(resultConstraints)
                .face_patch_map(resultCharts)
                .collapse_constraints(false)
                .do_project(true));
        if (result.is_empty() || !CGAL::is_triangle_mesh(result) || !CGAL::is_valid_polygon_mesh(result))
            throw std::runtime_error("invalid remeshing result");

        std::vector<std::vector<FACE>> chartFaces(chartCount);
        std::vector<std::string> materials(chartCount);
        std::vector<bool> materialAssigned(chartCount, false);
        for (FACE face : source.faces())
        {
            const auto chart = get(chartMap, face);
            chartFaces.at(chart).push_back(face);
            if (!materialAssigned[chart])
            {
                materials[chart] = input.faces.at(face.idx()).material;
                materialAssigned[chart] = true;
            }
        }
        std::vector<std::unique_ptr<TREE>> trees;
        trees.reserve(chartCount);
        for (const auto &facesInChart : chartFaces)
        {
            if (facesInChart.empty()) throw std::runtime_error("empty UV/material chart");
            auto tree = std::make_unique<TREE>(facesInChart.begin(), facesInChart.end(), source);
            tree->accelerate_distance_queries();
            trees.push_back(std::move(tree));
        }

        const bool sourceIntersects = PMP::does_self_intersect(source);
        const bool resultIntersects = PMP::does_self_intersect(result);
        auto sourceCopy = source;
        auto resultCopy = result;
        const auto sourceComponents = components(sourceCopy), resultComponents = components(resultCopy);
        const bool sourceClosed = CGAL::is_closed(source), resultClosed = CGAL::is_closed(result);
        const double sampledError = std::max(sampledDistance(source, result), sampledDistance(result, source));
        writeObj(argv[2], result, input, resultCharts, materials, trees, source);

        std::ostringstream report;
        report << std::setprecision(17)
               << "CGAL_REMESH_RESULT source_vertices=" << num_vertices(source)
               << " source_triangles=" << num_faces(source)
               << " result_vertices=" << num_vertices(result)
               << " result_triangles=" << num_faces(result)
               << " target_edge_length=" << targetEdgeLength
               << " charts=" << chartCount << " iterations=" << iterations
               << " source_closed=" << sourceClosed << " result_closed=" << resultClosed
               << " source_self_intersections=" << sourceIntersects
               << " result_self_intersections=" << resultIntersects
               << " source_components=" << sourceComponents << " result_components=" << resultComponents
               << " sampled_bidirectional_error=" << sampledError
               << " sampled_error_fraction=" << sampledError / boundsDiagonal
               << " seconds=" << std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count() << '\n';
        std::cout << report.str();
        if (reportFile.is_open() && !(reportFile << report.str())) throw std::runtime_error("cannot write report");
        std::string failure;
        if (resultIntersects && !sourceIntersects) failure = "result introduces self-intersections";
        else if (sourceComponents != resultComponents) failure = "result changes connected-component count";
        else if (sourceClosed != resultClosed) failure = "result changes closedness";
        if (!failure.empty())
        {
            std::cerr << "CGAL_FAIL " << failure << '\n';
            if (reportFile.is_open()) reportFile << "CGAL_FAIL " << failure << '\n';
            return 3;
        }
        return 0;
    }
    catch (const std::exception &error)
    {
        if (reportFile.is_open()) reportFile << "CGAL_FAIL " << error.what() << '\n';
        std::cerr << "CGAL_FAIL " << error.what() << '\n';
        return 1;
    }
}
