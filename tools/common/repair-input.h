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


#ifndef MBM_CGAL_REPAIR_INPUT_H
#define MBM_CGAL_REPAIR_INPUT_H
#include "../planar/uv-transfer.h"
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

namespace mbm_cgal_uv
{
struct REPAIR_REPORT
{
    std::size_t splitVertices = 0;
    std::size_t reversedFaces = 0;
};
// Keep every nondegenerate triangle and its corner attributes. Only connectivity
// and winding change. This is not hole filling or self-intersection removal.
inline REPAIR_REPORT repair(INPUT &input, MESH &mesh)
{
    std::vector<KERNEL::Point_3> points;
    for (auto vertex : mesh.vertices()) points.push_back(mesh.point(vertex));
    std::vector<std::vector<std::size_t>> polygons;
    for (const auto &face : input.faces)
    {
        const auto a = face.vertices[0].idx(), b = face.vertices[1].idx(), c = face.vertices[2].idx();
        if (CGAL::collinear(points[a], points[b], points[c]))
            throw std::runtime_error("input contains a degenerate triangle; topology repair does not remove faces");
        polygons.push_back({a,b,c});
    }
    const auto originalPoints = points.size();
    // False means points were duplicated, which is the requested repair.
    CGAL::Polygon_mesh_processing::orient_polygon_soup(points, polygons);
    if (!CGAL::Polygon_mesh_processing::is_polygon_soup_a_polygon_mesh(polygons))
        throw std::runtime_error("topology repair could not produce an oriented manifold");
    REPAIR_REPORT report;
    report.splitVertices = points.size() - originalPoints;
    // Use each triangle's exact positions to follow corner permutations and
    // duplicated indices; no approximate matching or cross-face UV lookup.
    for (std::size_t i = 0; i < polygons.size(); ++i)
    {
        auto &face = input.faces[i];
        const FACE original = face;
        std::array<unsigned, 3> order{};
        for (unsigned corner = 0; corner < 3; ++corner)
        {
            const auto id = polygons[i][corner];
            unsigned previous = 0;
            while (previous < 3 && points[id] != mesh.point(original.vertices[previous])) ++previous;
            if (previous == 3) throw std::runtime_error("topology repair changed a source position");
            order[corner] = previous;
            face.vertices[corner] = MESH::Vertex_index(static_cast<MESH::size_type>(id));
            face.uv[corner] = original.uv[previous];
        }
        if (order[1] != (order[0] + 1) % 3) ++report.reversedFaces;
    }
    mesh.clear();
    for (const auto &point : points) mesh.add_vertex(point);
    for (std::size_t i = 0; i < input.faces.size(); ++i)
    {
        const auto &face = input.faces[i];
        const auto f = mesh.add_face(face.vertices[0], face.vertices[1], face.vertices[2]);
        if (f == MESH::null_face() || f.idx() != i)
            throw std::runtime_error("could not construct repaired manifold");
    }
    return report;
}
}
#endif
