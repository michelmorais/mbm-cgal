# mbm-cgal-remesh

Runs CGAL Polygon Mesh Processing isotropic remeshing on a static triangular
OBJ mesh. Choose a uniform edge length or an approximate triangle target;
the output may contain more or fewer triangles than the input.

```sh
mbm-cgal-remesh input.obj output.obj 0.03 3 45 report.txt
```

Arguments: input OBJ, new output OBJ, target edge length as a fraction of the
input bounding-box diagonal (greater than 0 and at most 0.25), iterations (1..10,
default `3`), sharp-feature angle in degrees (0..180, default `45`), and an
optional new report path. Existing outputs and reports are refused.

Without repair enabled, input must be an oriented manifold triangle mesh with a UV on every face
corner. Exact-position welding builds geometric connectivity. Material changes,
UV seams, open boundaries and detected sharp edges constrain the remesher and
separate UV/material charts. Output UVs are interpolated from the closest point
on the corresponding source chart. Normals are not transferred; clients should
recompute shading normals. MTL references are retained; MTL files and textures
are not copied.

By default the worker rejects invalid/non-manifold input. Optional topology
repair is described below. The report
contains source/result counts, target edge length, chart count, topology checks,
sampled bidirectional deviation and duration. The deviation is sampled, not a
certified Hausdorff bound. Exit code `3` means the result introduces
self-intersections or changes connected-component count or closedness; clients
should not apply that result automatically. The structured result line is followed
by a `CGAL_FAIL` diagnostic explaining the rejection. A successful run may increase
triangle count. Fine or thin features can require a smaller target edge length or
feature angle; angle `0` constrains all source edges.

This is geometric remeshing, not semantic retopology: it does not create quad
edge flow, bake textures, transfer physics shapes, skin weights or animation.
The mini-mbm and mesh3dgen adapters preserve authored physics shapes separately.
Keep the worker outside the mini-mbm runtime and use it only for static offline
assets.
Run its focused test from the repository root with
`ctest --test-dir build --output-on-failure -R isotropic_remesh`.

## Approximate triangle target

```sh
mbm-cgal-remesh input.obj output.obj 0.03 3 45 report.txt --target-triangles 3000
```

The optional trailing flag takes an integer from 2 to 100000. Supply all six
positional arguments with this flag. The edge fraction remains validated for
compatibility but does not choose the initial length in target mode.
The worker measures source surface area A and estimates
`L = sqrt(4 * A / (sqrt(3) * target))`. It performs at most eight trials,
each starting from the original geometry, adjusting the length using measured
triangle counts and a bracket when available. It retains the topology-safe
candidate closest to the requested count, stopping early within 5%.
Trial lengths have a lower bound based on a 200000-triangle area estimate and
an upper bound of twice the input diagonal; these are search bounds, not hard
memory or output-count limits. Existing import vertex limits still apply.

This is a heuristic, not an exact count or a proof of optimality. Constraints
and discrete changes in triangulation may prevent convergence even for simple
meshes. No constraints are relaxed to force a match. No QEM pass is added.
A valid result outside tolerance exits 0 with `target_reached=0`; no valid
candidate fails without applying geometry. Check the report instead of assuming
that process success means the budget was met.

Additional report fields: `target_triangles` (0 for length mode),
`target_relative_error`, `target_reached` (0/1), `target_tolerance` (0.05),
`search_attempts`, and `surface_area`. Result counts use live vertices/faces,
excluding removed Surface_mesh storage slots.

All three editors expose a target checkbox and fixed-width integer field;
length mode remains the default for existing projects. Reports show requested
and achieved counts, the percentage difference, and a warning outside 5%.
Mesh Debug distributes a selected-subset total proportionally to source triangle
counts with at least two triangles per selected subset. Image Mesh applies the
target per generated region; mesh3dgen applies it to the selected local asset.

## Optional topology repair

```sh
mbm-cgal-remesh input.obj output.obj 0.03 3 45 report.txt --target-triangles 20000 --repair-topology
```

`--repair-topology` is opt-in and requires all six positional arguments. It can
also be used without `--target-triangles`. Before remeshing, CGAL orients the
triangle soup and duplicates vertices to split non-manifold connections. No
source triangles are removed and no source positions are moved during repair;
corner UVs and materials follow the repaired faces. Degenerate faces remain an
error. This does not fill holes or remove self-intersections. Repair may open
seams or separate components, and existing self-intersections may remain.

`repair_enabled`, `repair_split_vertices` and `repair_reversed_faces` are numeric
result fields. Source topology metrics and all subsequent topology comparisons
refer to the **repaired** source, not the original invalid soup. Output vertex
indices encode the splits: consumers must not weld coincident output vertices
back together when importing. The planar worker retains its strict input policy.
