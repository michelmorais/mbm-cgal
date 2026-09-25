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


#ifndef MBM_CGAL_UV_TRANSFER_H
#define MBM_CGAL_UV_TRANSFER_H
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mbm_cgal_uv
{
using KERNEL = CGAL::Exact_predicates_inexact_constructions_kernel;
using MESH = CGAL::Surface_mesh<KERNEL::Point_3>;
using UV = std::array<double, 2>;
using POSITION = std::array<double, 3>;
inline POSITION position(const KERNEL::Point_3 &p) { return {p.x(), p.y(), p.z()}; }
struct FACE
{
    std::array<MESH::Vertex_index, 3> vertices;
    std::array<UV, 3> uv;
    std::string material;
};
struct INPUT
{
    std::vector<FACE> faces;
    std::vector<std::string> libraries;
};
inline std::string rest(std::istringstream &stream)
{
    std::string value; std::getline(stream >> std::ws, value);
    const auto last = value.find_last_not_of(" \t\r");
    if (last == std::string::npos) return {};
    return value.substr(0, last + 1);
}
inline std::size_t index(const std::string &token, std::size_t count)
{
    std::size_t used = 0;
    const auto value = std::stoll(token, &used);
    const auto resolved = value < 0 ? static_cast<long long>(count) + value : value - 1;
    if (used != token.size() || value == 0 || resolved < 0 || static_cast<std::size_t>(resolved) >= count)
        throw std::runtime_error("OBJ index out of range");
    return static_cast<std::size_t>(resolved);
}
inline INPUT read(const std::string &path, MESH &mesh)
{
    std::ifstream file(path);
    if (!file) throw std::runtime_error("cannot open OBJ");
    INPUT input;
    std::vector<MESH::Vertex_index> vertices;
    std::map<POSITION, MESH::Vertex_index> welded;
    std::vector<UV> texcoords;
    std::size_t normalCount = 0;
    std::string line, material;
    while (std::getline(file, line))
    {
        if (const auto comment = line.find('#'); comment != std::string::npos) line.resize(comment);
        std::istringstream stream(line);
        std::string tag; if (!(stream >> tag)) continue;
        if (tag == "v" || tag == "vn")
        {
            double x, y, z;
            if (!(stream >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
                throw std::runtime_error("invalid OBJ position/normal");
            if (tag == "vn") { ++normalCount; continue; }
            std::string extra;
            if (stream >> extra) throw std::runtime_error("OBJ homogeneous/color vertices are not supported");
            const POSITION key{x, y, z};
            auto found = welded.find(key);
            if (found == welded.end()) found = welded.emplace(key, mesh.add_vertex(KERNEL::Point_3(x,y,z))).first;
            vertices.push_back(found->second);
        }
        else if (tag == "vt")
        {
            double u, v;
            if (!(stream >> u >> v) || !std::isfinite(u) || !std::isfinite(v))
                throw std::runtime_error("invalid OBJ UV");
            std::string extra;
            if (stream >> extra) throw std::runtime_error("only two-component OBJ UVs are supported");
            texcoords.push_back({u,v});
        }
        else if (tag == "usemtl") material = rest(stream);
        else if (tag == "mtllib")
        {
            const std::string library = rest(stream);
            if (library.empty()) throw std::runtime_error("empty OBJ material library");
            // One library path per line; preserve it relative to the source, not the output.
            input.libraries.push_back(std::filesystem::absolute(std::filesystem::path(path).parent_path() / library).lexically_normal().string());
        }
        else if (tag == "f")
        {
            FACE face; face.material = material;
            for (unsigned k = 0; k < 3; ++k)
            {
                std::string corner;
                if (!(stream >> corner)) throw std::runtime_error("OBJ requires triangular faces");
                const auto slash = corner.find('/');
                if (slash == std::string::npos) throw std::runtime_error("each OBJ corner requires a UV index");
                const auto second = corner.find('/', slash + 1);
                face.vertices[k] = vertices.at(index(corner.substr(0,slash), vertices.size()));
                face.uv[k] = texcoords.at(index(corner.substr(slash+1, second == std::string::npos ? second : second-slash-1), texcoords.size()));
                if (second != std::string::npos) index(corner.substr(second+1), normalCount);
            }
            std::string extra;
            if (stream >> extra) throw std::runtime_error("OBJ requires triangular faces");
            const auto f = mesh.add_face(face.vertices[0], face.vertices[1], face.vertices[2]);
            if (f == MESH::null_face()) throw std::runtime_error("OBJ is not an oriented manifold after exact-position welding");
            if (f.idx() != input.faces.size()) throw std::runtime_error("unexpected face indexing");
            input.faces.push_back(face);
        }
        else if (tag != "o" && tag != "g" && tag != "s")
            throw std::runtime_error("unsupported OBJ record: " + tag);
    }
    return input;
}

struct CHART
{
    int drop = 2;
    std::array<long double, 2> origin{}, b{}, c{};
    long double determinant = 0;
    std::array<UV, 3> uv{};
    std::string material;
    KERNEL::Vector_3 normal;
    std::map<POSITION, UV> corners;
    std::array<long double, 2> project(const KERNEL::Point_3 &p) const
    {
        const auto v = position(p);
        return {v[(drop+1)%3], v[(drop+2)%3]};
    }
    UV evaluate(const KERNEL::Point_3 &p) const
    {
        const auto q = project(p);
        const auto x = q[0]-origin[0], y = q[1]-origin[1];
        if (x==0 && y==0) return uv[0];
        if (x==b[0] && y==b[1]) return uv[1];
        if (x==c[0] && y==c[1]) return uv[2];
        const auto w1 = (x*c[1]-y*c[0])/determinant;
        const auto w2 = (b[0]*y-b[1]*x)/determinant;
        UV result;
        for (unsigned k=0; k<2; ++k)
            result[k] = static_cast<double>(uv[0][k]+w1*(static_cast<long double>(uv[1][k])-uv[0][k])+w2*(static_cast<long double>(uv[2][k])-uv[0][k]));
        return result;
    }
};
inline CHART chart(const FACE &face, const MESH &mesh, const KERNEL::Vector_3 &normal)
{
    CHART result; result.normal=normal; result.material=face.material; result.uv=face.uv;
    const std::array<double,3> n{std::abs(normal.x()),std::abs(normal.y()),std::abs(normal.z())};
    result.drop=static_cast<int>(std::max_element(n.begin(), n.end())-n.begin());
    result.origin=result.project(mesh.point(face.vertices[0]));
    auto b=result.project(mesh.point(face.vertices[1])), c=result.project(mesh.point(face.vertices[2]));
    for (unsigned k=0;k<2;++k) { result.b[k]=b[k]-result.origin[k]; result.c[k]=c[k]-result.origin[k]; }
    result.determinant=result.b[0]*result.c[1]-result.b[1]*result.c[0];
    return result;
}
inline const UV &at(const FACE &face, MESH::Vertex_index vertex)
{
    for (unsigned k=0;k<3;++k) if (face.vertices[k]==vertex) return face.uv[k];
    throw std::runtime_error("UV vertex not on source face");
}
struct CHARTS
{
    std::vector<CHART> values;
    std::size_t seamEdges=0, materialEdges=0;
    double maximumResidual=0;
};
// Split each geometric patch into connected affine UV charts. A fixed field from
// the largest available projected triangle prevents accumulated pairwise drift.
// The output retains UV seams independently of shared geometric vertices.
template<typename NormalMap>
CHARTS split(const MESH &mesh, const INPUT &input, std::vector<std::size_t> &regions,
             NormalMap &normals, double epsilon)
{
    CHARTS output;
    const auto geometricRegions=regions;
    const std::size_t unassigned=std::size_t(-1);
    std::fill(regions.begin(), regions.end(), unassigned);
    std::vector<std::vector<std::size_t>> neighbors(input.faces.size());
    for (auto edge:mesh.edges())
    {
        const auto h=mesh.halfedge(edge), opposite=mesh.opposite(h);
        const auto a=mesh.face(h), b=mesh.face(opposite);
        if (a==MESH::null_face() || b==MESH::null_face()) continue;
        const auto &fa=input.faces[a.idx()], &fb=input.faces[b.idx()];
        const bool material=fa.material!=fb.material;
        const bool seam=at(fa,mesh.source(h))!=at(fb,mesh.source(h)) || at(fa,mesh.target(h))!=at(fb,mesh.target(h));
        output.materialEdges+=material; output.seamEdges+=seam;
        if (!material && !seam && geometricRegions[a.idx()]==geometricRegions[b.idx()])
        { neighbors[a.idx()].push_back(b.idx()); neighbors[b.idx()].push_back(a.idx()); }
    }
    std::vector<std::pair<long double,std::size_t>> seeds;
    for (std::size_t f=0;f<input.faces.size();++f)
    {
        const auto field=chart(input.faces[f],mesh,get(normals,geometricRegions[f]));
        if (field.determinant==0) throw std::runtime_error("UV patch projection degenerates; reduce geometric tolerance");
        seeds.emplace_back(std::abs(field.determinant),f);
    }
    std::sort(seeds.begin(),seeds.end(),[](const auto &a,const auto &b) {
        return a.first!=b.first ? a.first>b.first : a.second<b.second;
    });
    for (const auto &seed:seeds)
    {
        if (regions[seed.second]!=unassigned) continue;
        CHART field=chart(input.faces[seed.second],mesh,get(normals,geometricRegions[seed.second]));
        const auto id=output.values.size();
        std::vector<std::size_t> queue{seed.second};
        for (std::size_t head=0;head<queue.size();++head)
        {
            const auto f=queue[head];
            if (regions[f]!=unassigned) continue;
            const auto &face=input.faces[f];
            double residual=0; bool conflict=false;
            for (unsigned k=0;k<3;++k)
            {
                const auto &p=mesh.point(face.vertices[k]);
                const auto expected=field.evaluate(p);
                for (unsigned axis=0;axis<2;++axis)
                {
                    if (!std::isfinite(expected[axis])) throw std::runtime_error("non-finite UV field");
                    residual=std::max(residual,std::abs(expected[axis]-face.uv[k][axis]));
                }
                const auto found=field.corners.find(position(p));
                conflict=conflict || (found!=field.corners.end() && found->second!=face.uv[k]);
            }
            if (conflict || residual>epsilon) continue;
            regions[f]=id;
            output.maximumResidual=std::max(output.maximumResidual,residual);
            for (unsigned k=0;k<3;++k) field.corners.emplace(position(mesh.point(face.vertices[k])),face.uv[k]);
            for (auto next:neighbors[f]) if (regions[next]==unassigned) queue.push_back(next);
        }
        if (regions[seed.second]==unassigned) throw std::runtime_error("UV seed residual exceeds tolerance");
        output.values.push_back(std::move(field));
    }
    // Do this after reading all original region normals: IDs now refer to UV charts.
    for (std::size_t i=0;i<output.values.size();++i) put(normals,i,output.values[i].normal);
    return output;
}

template<typename PatchMap>
void write(const std::string &path, const MESH &mesh, const INPUT &input,
           const CHARTS &charts, const PatchMap &patches)
{
    // Prepare and validate before creating the output file.
    std::ostringstream out; out<<std::setprecision(17);
    out<<"# mini-mbm CGAL UV remesh; source normals are not transferred\n";
    for (const auto &library:input.libraries) out<<"mtllib "<<library<<'\n';
    std::map<MESH::Vertex_index,std::size_t> ids;
    for (auto v:mesh.vertices())
    {
        ids[v]=ids.size()+1;
        const auto &p=mesh.point(v);out<<"v "<<p.x()<<' '<<p.y()<<' '<<p.z()<<'\n';
    }
    std::size_t uvIndex=1;
    for (auto f:mesh.faces())
    {
        const auto &field=charts.values.at(get(patches,f));
        out<<"usemtl "<<field.material<<'\n';
        std::array<std::size_t,3> vertexIds{};
        auto h=mesh.halfedge(f);
        for (unsigned k=0;k<3;++k,h=mesh.next(h))
        {
            const auto v=mesh.source(h);vertexIds[k]=ids.at(v);
            const auto found=field.corners.find(position(mesh.point(v)));
            if (found==field.corners.end()) throw std::runtime_error("CGAL output corner has no source UV in its patch");
            out<<"vt "<<found->second[0]<<' '<<found->second[1]<<'\n';
        }
        out<<'f';for (auto id:vertexIds) out<<' '<<id<<'/'<<uvIndex++;
        out<<'\n';
    }
    std::ofstream file(path);
    if (!file || !(file<<out.str())) throw std::runtime_error("cannot write OBJ output");
}
}
#endif
