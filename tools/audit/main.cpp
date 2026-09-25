// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Michel Braz de Morais
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/IO/polygon_soup_io.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include "../common/mesh-audit.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <limits>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace PMP = CGAL::Polygon_mesh_processing;
using KERNEL = CGAL::Exact_predicates_inexact_constructions_kernel;
using POINT = KERNEL::Point_3;
using MESH = CGAL::Surface_mesh<POINT>;
using POLYGON = std::vector<std::size_t>;

static std::string quote(const std::string &value)
{
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value)
    {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(c);
        else out << c;
    }
    out << '"';
    return out.str();
}
struct REPORT
{
    std::ostringstream out;
    bool first = true;
    REPORT() { out << std::setprecision(17) << "{\n"; }
    void raw(const std::string &key, const std::string &value)
    {
        if (!first) out << ",\n";
        first = false;
        out << "  " << quote(key) << ": " << value;
    }
    void text(const std::string &key, const std::string &value) { raw(key, quote(value)); }
    void flag(const std::string &key, bool value) { raw(key, value ? "true" : "false"); }
    template<class T> void number(const std::string &key, T value)
    {
        if (!std::isfinite(static_cast<double>(value))) throw std::runtime_error("numeric range exceeded");
        std::ostringstream s; s << std::setprecision(17) << value; raw(key, s.str());
    }
    std::string finish() { return out.str() + "\n}\n"; }
};
struct STATS
{
    double min = std::numeric_limits<double>::infinity(), max = 0, sum = 0;
    std::size_t count = 0;
    void add(double value) { min = std::min(min, value); max = std::max(max, value); sum += value; ++count; }
    void write(REPORT &r, const std::string &prefix) const
    {
        if (!count) { r.raw(prefix+"_min", "null"); r.raw(prefix+"_max", "null"); r.raw(prefix+"_mean", "null"); return; }
        r.number(prefix+"_min", min); r.number(prefix+"_max", max); r.number(prefix+"_mean", sum/count);
    }
};

static std::string audit(const std::string &path, bool intersections)
{
    std::vector<POINT> rawPoints, points;
    std::vector<POLYGON> faces;
    if (!CGAL::IO::read_polygon_soup(path, rawPoints, faces)) throw std::runtime_error("cannot read OBJ/OFF polygon soup");
    std::map<std::array<double, 3>, std::size_t> unique;
    std::vector<std::size_t> remap;
    for (const POINT &p : rawPoints)
    {
        const std::array<double, 3> xyz{p.x(), p.y(), p.z()};
        for (double v : xyz) if (!std::isfinite(v)) throw std::runtime_error("non-finite position");
        auto inserted = unique.emplace(xyz, points.size());
        if (inserted.second) points.push_back(p);
        remap.push_back(inserted.first->second);
    }
    for (auto &face : faces) for (auto &v : face) v = remap.at(v);
    REPORT r;
    r.number("schema_version", 1); r.text("operation", "mesh_audit"); r.text("status", "completed");
    r.text("welding", "exact_position"); r.text("attributes_status", "not_checked");
    r.number("input_vertices", rawPoints.size()); r.number("vertices", points.size());
    r.number("duplicate_positions", rawPoints.size()-points.size()); r.number("faces", faces.size());
    if (points.empty())
    {
        for (auto key : {"bounds_min_x", "bounds_min_y", "bounds_min_z", "bounds_max_x", "bounds_max_y", "bounds_max_z", "diagonal"}) r.raw(key,"null");
    }
    else
    {
        auto box = points.front().bbox();
        for (const auto &p : points) box = box + p.bbox();
        r.number("bounds_min_x",box.xmin()); r.number("bounds_min_y",box.ymin()); r.number("bounds_min_z",box.zmin());
        r.number("bounds_max_x",box.xmax()); r.number("bounds_max_y",box.ymax()); r.number("bounds_max_z",box.zmax());
        r.number("diagonal",std::hypot(box.xmax()-box.xmin(),box.ymax()-box.ymin(),box.zmax()-box.zmin()));
    }
    using EDGE = std::pair<std::size_t,std::size_t>;
    struct USE { std::size_t face; bool forward; };
    std::map<EDGE,std::vector<USE>> edges;
    std::vector<std::size_t> parent(faces.size()); std::iota(parent.begin(),parent.end(),0);
    const auto root = [&parent](std::size_t n) { while (parent[n]!=n) { parent[n]=parent[parent[n]]; n=parent[n]; } return n; };
    std::set<std::size_t> used;
    std::set<POLYGON> seen;
    std::size_t triangles=0, degenerate=0, duplicates=0;
    STATS areas, quality, lengths;
    double minAngle=180;
    for (std::size_t f=0; f<faces.size(); ++f)
    {
        const auto &face=faces[f];
        POLYGON sorted=face; std::sort(sorted.begin(),sorted.end());
        if (!seen.insert(sorted).second) ++duplicates;
        for (std::size_t i=0;i<face.size();++i)
        {
            const auto a=face[i], b=face[(i+1)%face.size()]; used.insert(a);
            auto &incidence=edges[std::minmax(a,b)];
            if (!incidence.empty()) parent[root(f)]=root(incidence.front().face);
            incidence.push_back({f,a<b});
        }
        if (face.size()!=3) continue;
        ++triangles;
        const auto &a=points[face[0]], &b=points[face[1]], &c=points[face[2]];
        const bool flat=CGAL::collinear(a,b,c);
        if (flat) ++degenerate;
        const double area=std::sqrt(CGAL::to_double(CGAL::cross_product(b-a,c-a).squared_length()))*.5;
        areas.add(area);
        const std::array<double,3> squared{CGAL::to_double(CGAL::squared_distance(a,b)),CGAL::to_double(CGAL::squared_distance(b,c)),CGAL::to_double(CGAL::squared_distance(c,a))};
        const double sum=squared[0]+squared[1]+squared[2];
        quality.add(sum>0 ? 4*std::sqrt(3.)*area/sum : 0);
        if (flat) { minAngle=0; continue; }
        for (unsigned i=0;i<3;++i)
        {
            const double x=squared[i],y=squared[(i+1)%3],z=squared[(i+2)%3];
            const double cosine=std::clamp((x+y-z)/(2*std::sqrt(x*y)),-1.,1.);
            minAngle=std::min(minAngle,std::acos(cosine)*180/std::acos(-1.));
        }
    }
    std::size_t boundary=0, nonmanifold=0, orientation=0;
    for (const auto &entry : edges)
    {
        const auto &incidence=entry.second;
        if (incidence.size()==1) ++boundary;
        if (incidence.size()>2) ++nonmanifold;
        if (incidence.size()==2 && incidence[0].forward==incidence[1].forward) ++orientation;
        lengths.add(std::sqrt(CGAL::to_double(CGAL::squared_distance(points[entry.first.first],points[entry.first.second]))));
    }
    std::set<std::size_t> components;
    for (std::size_t i=0;i<faces.size();++i) components.insert(root(i));
    const bool valid=!faces.empty() && PMP::is_polygon_soup_a_polygon_mesh(faces);
    r.number("triangles",triangles); r.number("non_triangle_faces",faces.size()-triangles);
    r.number("degenerate_triangles",degenerate); r.number("duplicate_faces",duplicates);
    r.number("isolated_vertices",points.size()-used.size()); r.number("edges",edges.size());
    r.number("boundary_edges",boundary); r.number("non_manifold_edges",nonmanifold); r.number("orientation_conflicts",orientation);
    r.number("components",components.size()); r.flag("valid_polygon_mesh",valid);
    if (valid) r.flag("closed",boundary==0); else r.raw("closed","null");
    const bool triangular=triangles==faces.size() && !faces.empty();
    r.text("triangle_metrics_status", triangular ? "completed" : "partial_triangles_only");
    if (triangular) r.number("surface_area",areas.sum); else r.raw("surface_area","null");
    areas.write(r,"triangle_area"); quality.write(r,"triangle_quality"); lengths.write(r,"edge_length");
    if (triangles) r.number("minimum_angle_degrees",minAngle); else r.raw("minimum_angle_degrees","null");
    std::string check=!intersections ? "not_requested" : !valid ? "skipped_invalid_topology" : !triangular ? "skipped_non_triangles" : degenerate ? "skipped_degenerate_triangles" : "completed";
    r.text("self_intersections_status",check);
    if (check=="completed")
    {
        MESH mesh; PMP::polygon_soup_to_polygon_mesh(points,faces,mesh);
        r.flag("has_self_intersections",mbm_cgal_audit::topology(mesh).selfIntersects);
    }
    else r.raw("has_self_intersections","null");
    return r.finish();
}
int main(int argc,char **argv)
{
    std::string reportPath;
    bool canWriteReport=false;
    try
    {
        if (argc<2) throw std::runtime_error("usage: mbm-cgal-audit input.obj|input.off [--report new.json] [--skip-self-intersections]");
        bool intersections=true;
        for (int i=2;i<argc;++i)
        {
            const std::string arg=argv[i];
            if (arg=="--skip-self-intersections") intersections=false;
            else if (arg=="--report" && i+1<argc && reportPath.empty()) reportPath=argv[++i];
            else throw std::runtime_error("invalid arguments");
        }
        if (!reportPath.empty() && (std::filesystem::exists(reportPath) || std::filesystem::is_symlink(reportPath))) throw std::runtime_error("report already exists");
        if (!reportPath.empty() && std::filesystem::absolute(reportPath).lexically_normal()==std::filesystem::absolute(argv[1]).lexically_normal())
            throw std::runtime_error("report must differ from input");
        canWriteReport=!reportPath.empty();
        auto extension=std::filesystem::path(argv[1]).extension().string();
        std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c){return std::tolower(c);});
        if (extension!=".obj" && extension!=".off") throw std::runtime_error("input must be OBJ or OFF");
        const auto result=audit(argv[1],intersections);
        if (!reportPath.empty())
        {
            std::ofstream file(reportPath,std::ios::binary);
            if (!file || !(file<<result)) throw std::runtime_error("cannot write report");
            file.close(); if (!file) throw std::runtime_error("cannot close report");
        }
        std::cout<<result;
        return 0;
    }
    catch (const std::exception &error)
    {
        REPORT r; r.number("schema_version",1); r.text("operation","mesh_audit"); r.text("status","failed"); r.text("error",error.what());
        const auto json=r.finish();
        if (canWriteReport) { std::ofstream file(reportPath,std::ios::binary); if (file) file<<json; }
        std::cerr<<json; return 1;
    }
}
