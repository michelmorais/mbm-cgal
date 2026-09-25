# mbm-cgal-audit

Read-only geometric analysis of OBJ/OFF polygon soups. No repair, triangulation,
normalization or output mesh is performed. UVs and materials are not required.

```sh
mbm-cgal-audit input.obj
mbm-cgal-audit input.off --report new-report.json
mbm-cgal-audit input.obj --report new-report.json --skip-self-intersections
```

The worker always writes the completed JSON to stdout. `--report` also writes a
new file; existing files are refused. Exit `0` means analysis completed, **not**
that the mesh has no defects. Exit `1` means an argument, input, numeric-range or
I/O failure. Failures use JSON on stderr and, when the new report path was
accepted and can be written, in that report file. No source file is modified.

## JSON API, schema version 1

The report is a flat object with scalar values. Clients must check
`schema_version == 1`, `operation == "mesh_audit"` and `status`. Successful
reports have `status: "completed"`; failures have `status: "failed"` and an
`error` string. Field order is not an API guarantee. Unknown fields can be ignored.

| Fields | Meaning |
|---|---|
| `welding` | `exact_position`: geometry is analyzed after merging exactly equal XYZ positions in an internal copy |
| `input_vertices`, `vertices`, `duplicate_positions` | Position records, unique positions, and their difference |
| `faces`, `triangles`, `non_triangle_faces` | Polygon and triangular-face counts |
| `bounds_min_x/y/z`, `bounds_max_x/y/z`, `diagonal` | Bounds of all unique positions, including isolated vertices; null for no positions |
| `edges`, `boundary_edges`, `non_manifold_edges` | Unique unordered edges; incidence count 1; incidence count greater than 2 |
| `orientation_conflicts` | Edges with exactly two incident face edges traversed in the same direction |
| `components` | Face components connected by a shared geometric edge; isolated vertices are excluded |
| `isolated_vertices` | Unique positions referenced by no face |
| `duplicate_faces` | Repeated unordered face vertex sets, independent of winding |
| `valid_polygon_mesh` | Nonempty soup satisfies CGAL's oriented manifold polygon-mesh topology predicate, including vertex manifoldness |
| `closed` | No boundary edges for valid topology; null when topology is invalid |
| `degenerate_triangles` | Triangles whose geometric points are collinear, including coincident corners |
| `surface_area` | Sum of triangle areas when all faces are triangles and the mesh is nonempty; otherwise null |
| `triangle_area_min/max/mean` | Statistics over triangular faces only; null when none exist |
| `triangle_quality_min/max/mean` | `4 * sqrt(3) * area / (a²+b²+c²)`: 1 for equilateral, 0 for degenerate triangles |
| `minimum_angle_degrees` | Smallest triangle angle; 0 for degenerates; null when there are no triangles |
| `edge_length_min/max/mean` | Length statistics over unique geometric edges; null when there are no edges |
| `triangle_metrics_status` | `completed` for nonempty all-triangle input; otherwise `partial_triangles_only` |
| `self_intersections_status` | See below |
| `has_self_intersections` | Boolean only when the test completed; otherwise null |
| `attributes_status` | `not_checked`: UVs, materials, normals, skin weights and animation are outside this geometric report |

Lengths and areas use the input's coordinate units and squared units. Exact
welding joins separate position records at the same XYZ, including render-vertex
splits at UV/normal seams. The original file and editor asset remain unchanged.
A topology-valid mesh may still contain degenerate triangles or self-intersections.
`non_manifold_edges == 0` alone does not prove vertex manifoldness.

Self-intersection status values:

- `completed`: tested with CGAL; use the boolean result.
- `not_requested`: disabled with the CLI option.
- `skipped_invalid_topology`: input cannot be represented by an oriented manifold polygon mesh.
- `skipped_non_triangles`: some faces are not triangles; the tool does not triangulate them.
- `skipped_degenerate_triangles`: unsafe preconditions for the intersection test.

The checks are geometric diagnostics, not proof that an asset is suitable for
rendering, physics, fabrication or a requested remeshing triangle budget. Repair,
volume, boundary-loop enumeration, per-defect highlighting, UV/material seam
analysis and triangle-target optimization are not implemented here.

## Integration and tests

`tools/common/mesh-audit.h` shares topology checks with `mbm-cgal-remesh`.
The Lua API `mini-mbm/editor/mesh_audit.lua` starts file or engine-mesh snapshot
analysis asynchronously. `mesh_audit_ui.lua` presents the same report in Mesh
Debug, Image Mesh Editor and mesh3dgen, with cancellation and JSON export.
See mini-mbm `docs/mesh-audit.md` for the public Lua API.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$PWD/install"
```

The `mesh_audit` CTest covers open/closed geometry, area and quality, degenerate
triangles, duplicate faces and positions, inconsistent winding, non-manifold
vertices/edges, self-intersection detection, skipped checks, polygons, OFF,
JSON parsing and source-file preservation.
