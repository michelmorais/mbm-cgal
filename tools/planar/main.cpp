/*-----------------------------------------------------------------------------------------------------------------------|
| MIT License (MIT)                                                                                                      |
| Copyright (C) 2026 by Michel Braz de Morais <michel.braz.morais@gmail.com>                                             |
|                                                                                                                        |
| Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated           |
| documentation files (the "Software"), to deal in the Software without restriction, including without limitation       |
| the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and       |
| to permit persons to whom the Software is furnished to do so, subject to the following conditions:                     |
| The above copyright notice and this permission notice shall be included in all copies or substantial portions.         |
| THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   |
| WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR  |
| COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR       |
| OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.       |
|-----------------------------------------------------------------------------------------------------------------------*/


// Optional standalone experiment. Does not link or modify the mini-mbm core.
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>
#include <CGAL/Polygon_mesh_processing/region_growing.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/boost/graph/IO/polygon_mesh_io.h>
#include <boost/property_map/vector_property_map.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include "uv-transfer.h"

namespace PMP = CGAL::Polygon_mesh_processing;
using KERNEL = CGAL::Exact_predicates_inexact_constructions_kernel;
using MESH = CGAL::Surface_mesh<KERNEL::Point_3>;

// Deterministic bidirectional sample estimate, not a certified Hausdorff bound.
static double sampledDistance(const MESH &source, const MESH &target)
{
    using PRIMITIVE = CGAL::AABB_face_graph_triangle_primitive<MESH>;
    using TRAITS = CGAL::AABB_traits_3<KERNEL, PRIMITIVE>;
    CGAL::AABB_tree<TRAITS> tree(faces(target).first, faces(target).second, target);
    tree.accelerate_distance_queries();
    double maximum = 0;
    for (auto f : source.faces())
    {
        const auto h = source.halfedge(f);
        const auto &a = source.point(source.source(h));
        const auto &b = source.point(source.target(h));
        const auto &c = source.point(source.target(source.next(h)));
        for (const auto &p : {a, b, c, CGAL::midpoint(a,b), CGAL::midpoint(b,c),
                              CGAL::midpoint(c,a), CGAL::centroid(a,b,c)})
            maximum = std::max(maximum, CGAL::to_double(tree.squared_distance(p)));
    }
    return std::sqrt(maximum);
}

static std::size_t components(MESH &mesh)
{
    auto map = mesh.add_property_map<MESH::Face_index, std::size_t>("f:component", 0).first;
    return PMP::connected_components(mesh, map);
}

#include <sstream>

int main(int argc, char **argv)
{
    if (argc != 5 && argc != 6 && argc != 7)
    {
        std::cerr << "Usage: mbm-cgal-planar input.off|obj output.off|obj angle-deg distance-fraction [uv-epsilon [report-path]]\n";
        return 2;
    }
    std::ofstream reportFile;
    try
    {
        if (argc==7 && std::filesystem::exists(argv[6])) throw std::runtime_error("report already exists");
        if (argc==7)
        {
            reportFile.open(argv[6]);
            if (!reportFile) throw std::runtime_error("cannot open report");
        }
        if (std::filesystem::exists(argv[2])) throw std::runtime_error("output already exists; choose a new file");
        const auto number = [](const char *value) {
            std::size_t consumed = 0;
            const double result = std::stod(value, &consumed);
            if (consumed != std::string(value).size()) throw std::runtime_error("invalid numeric argument");
            return result;
        };
        const double angle = number(argv[3]), fraction = number(argv[4]);
        if (!std::isfinite(angle) || angle < 0 || angle > 60 ||
            !std::isfinite(fraction) || fraction < 0 || fraction > 0.1)
            throw std::runtime_error("angle range: 0..60; distance fraction: 0..0.1");
        const auto extension=std::filesystem::path(argv[1]).extension();
        if (extension!=".off" && extension!=".obj") throw std::runtime_error("input must be .off or .obj");
        const bool withUv = extension==".obj";
        if (std::filesystem::path(argv[2]).extension() != (withUv ? ".obj" : ".off"))
            throw std::runtime_error("output extension must match input format");
        if (!withUv && argc>=6) throw std::runtime_error("UV tolerance requires OBJ input");
        const double uvEpsilon=argc>=6 ? number(argv[5]) : 1e-6;
        if (!std::isfinite(uvEpsilon) || uvEpsilon<0 || uvEpsilon>0.01)
            throw std::runtime_error("UV epsilon range: 0..0.01 UV units");
        MESH source;
        mbm_cgal_uv::INPUT uvInput;
        if (withUv) uvInput=mbm_cgal_uv::read(argv[1], source);
        else if (!CGAL::IO::read_polygon_mesh(argv[1], source)) throw std::runtime_error("cannot read OFF mesh");
        if (source.is_empty() ||
            !CGAL::is_triangle_mesh(source) || !CGAL::is_valid_polygon_mesh(source))
            throw std::runtime_error("input is not a valid manifold triangle mesh; no automatic repair performed");
        const auto begin = std::chrono::steady_clock::now();
        CGAL::Bbox_3 bounds;
        bool first = true;
        for (auto v : source.vertices())
        {
            if (first) { bounds = source.point(v).bbox(); first = false; }
            else bounds = bounds + source.point(v).bbox();
        }
        const double dx = bounds.xmax()-bounds.xmin(), dy = bounds.ymax()-bounds.ymin(), dz = bounds.zmax()-bounds.zmin();
        const double diagonal = std::sqrt(dx*dx+dy*dy+dz*dz);
        if (!std::isfinite(diagonal) || diagonal <= 0) throw std::runtime_error("invalid input bounds");
        const double distance = fraction * diagonal;
        const double cosine = std::cos(angle * std::acos(-1.0) / 180.0);
        std::vector<std::size_t> regions(num_faces(source));
        std::vector<std::size_t> corners(num_vertices(source), std::size_t(-1));
        std::vector<bool> constraints(num_edges(source), false);
        boost::vector_property_map<KERNEL::Vector_3> normals;
        const auto regionMap = CGAL::make_random_access_property_map(regions);
        const auto cornerMap = CGAL::make_random_access_property_map(corners);
        const auto edgeMap = CGAL::make_random_access_property_map(constraints);
        auto regionCount = PMP::region_growing_of_planes_on_faces(source, regionMap,
            CGAL::parameters::cosine_of_maximum_angle(cosine).maximum_distance(distance).region_primitive_map(normals));
        const auto geometricRegionCount=regionCount;
        mbm_cgal_uv::CHARTS uvCharts;
        if (withUv)
        {
            uvCharts=mbm_cgal_uv::split(source,uvInput,regions,normals,uvEpsilon);
            regionCount=uvCharts.values.size();
        }
        const auto cornerCount = PMP::detect_corners_of_regions(source, regionMap, regionCount, cornerMap,
            CGAL::parameters::cosine_of_maximum_angle(cosine).maximum_distance(distance).edge_is_constrained_map(edgeMap));
        MESH result;
        const auto resultPatches=result.add_property_map<MESH::Face_index,std::size_t>("f:patch",std::size_t(-1)).first;
        const bool remeshed = PMP::remesh_almost_planar_patches(source, result, regionCount, cornerCount,
            regionMap, cornerMap, edgeMap, CGAL::parameters::patch_normal_map(normals),
            CGAL::parameters::face_patch_map(resultPatches));
        if (result.is_empty() || !CGAL::is_triangle_mesh(result) || !CGAL::is_valid_polygon_mesh(result))
            throw std::runtime_error("invalid remeshing result");
        const bool sourceIntersects = PMP::does_self_intersect(source);
        const bool resultIntersects = PMP::does_self_intersect(result);
        const auto sourceComponents = components(source), resultComponents = components(result);
        const double sampledError = std::max(sampledDistance(source, result), sampledDistance(result, source));
        if (withUv) mbm_cgal_uv::write(argv[2],result,uvInput,uvCharts,resultPatches);
        else if (!CGAL::IO::write_polygon_mesh(argv[2], result, CGAL::parameters::stream_precision(17)))
            throw std::runtime_error("cannot write output");
        std::ostringstream report;
        report.precision(17);
        report << "CGAL_RESULT source_vertices=" << num_vertices(source) << " source_triangles=" << num_faces(source)
                  << " result_vertices=" << num_vertices(result) << " result_triangles=" << num_faces(result)
                  << " geometric_regions=" << geometricRegionCount << " regions=" << regionCount
                  << " uv_enabled=" << withUv << " uv_seam_edges=" << uvCharts.seamEdges
                  << " material_edges=" << uvCharts.materialEdges << " uv_maximum_residual=" << uvCharts.maximumResidual << " corners=" << cornerCount << " all_patches_remeshed=" << remeshed
                  << " source_closed=" << CGAL::is_closed(source) << " result_closed=" << CGAL::is_closed(result)
                  << " source_self_intersections=" << sourceIntersects << " result_self_intersections=" << resultIntersects
                  << " source_components=" << sourceComponents << " result_components=" << resultComponents
                  << " sampled_bidirectional_error=" << sampledError
                  << " sampled_error_fraction=" << sampledError / diagonal
                  << " seconds=" << std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count() << '\n';
        std::cout << report.str();
        if (argc==7)
        {
            if (!(reportFile << report.str())) throw std::runtime_error("cannot write report");
        }
        return (resultIntersects && !sourceIntersects) || sourceComponents != resultComponents ||
               CGAL::is_closed(source) != CGAL::is_closed(result) ? 3 : 0;
    }
    catch (const std::exception &e)
    {
        if (reportFile.is_open()) reportFile << "CGAL_FAIL " << e.what() << '\n';
        std::cerr << "CGAL_FAIL " << e.what() << '\n';
        return 1;
    }
}
