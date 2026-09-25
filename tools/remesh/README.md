# mbm-cgal-remesh

Runs CGAL Polygon Mesh Processing isotropic remeshing on a static triangular
OBJ mesh. It targets a uniform edge length, not a polygon count; the output may
contain more or fewer triangles than the input.

```sh
mbm-cgal-remesh input.obj output.obj 0.03 3 45 report.txt
```

Arguments: input OBJ, new output OBJ, target edge length as a fraction of the
input bounding-box diagonal (greater than 0 and at most 0.25), iterations (1..10,
default `3`), sharp-feature angle in degrees (0..180, default `45`), and an
optional new report path. Existing outputs and reports are refused.

The input must be an oriented manifold triangle mesh with a UV on every face
corner. Exact-position welding builds geometric connectivity. Material changes,
UV seams, open boundaries and detected sharp edges constrain the remesher and
separate UV/material charts. Output UVs are interpolated from the closest point
on the corresponding source chart. Normals are not transferred; clients should
recompute shading normals. MTL references are retained; MTL files and textures
are not copied.

The worker rejects invalid/non-manifold input and does not repair it. The report
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
