// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Michel Braz de Morais
#ifndef MBM_CGAL_MESH_AUDIT_H
#define MBM_CGAL_MESH_AUDIT_H

#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>

namespace mbm_cgal_audit
{
struct TOPOLOGY
{
    std::size_t components;
    bool closed;
    bool selfIntersects;
};
// Requires a valid, non-degenerate triangle mesh. Works on a copy so property
// maps used by connected_components never alter the caller's mesh.
template<class Mesh> TOPOLOGY topology(const Mesh &mesh, bool checkIntersections = true)
{
    Mesh copy = mesh;
    auto map = copy.template add_property_map<typename Mesh::Face_index, std::size_t>("f:audit_component", 0).first;
    const auto count = CGAL::Polygon_mesh_processing::connected_components(copy, map);
    return {count, CGAL::is_closed(mesh),
        checkIntersections && CGAL::Polygon_mesh_processing::does_self_intersect(mesh)};
}
}
#endif
