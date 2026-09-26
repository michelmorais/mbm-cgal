# mbm-cgal-planar

Uses CGAL plane region growing, corner detection and
`remesh_almost_planar_patches` to reduce a triangle mesh. Input must be manifold
after exact-position welding by default; optional topology preparation is explicit.
There is no approximate welding.

```sh
mbm-cgal-planar input.obj output.obj 5 0.01 0.000001 report.txt
mbm-cgal-planar input.off output.off 5 0.01
```

Arguments: input, new output, angle in degrees (0..60), plane distance as a
fraction of the input bounding-box diagonal (0..0.1), optional UV epsilon
(OBJ only, 0..0.01, default `1e-6`), optional new report path after UV epsilon.
Input/output extensions must match. Existing outputs and reports are refused.

OBJ accepts triangle faces with corner UVs, including negative indices. Exact
positions are welded for geometric topology; UV seams and material boundaries
remain separate. Geometric regions are split into connected affine UV charts.
UV fitting uses the largest projected triangle as a seed; the accepted residual
is limited by UV epsilon. Even smaller discontinuities across a seam remain
constraints. Output corners copy source UV values at their original positions
within the same chart. Material assignments and `mtllib` references are retained;
referenced MTL paths are made absolute. Textures and MTL contents are not copied.

Shading normals, skin weights and animation are not transferred. Input `vn` is
accepted but discarded; clients must decide how to reconstruct normals. OFF
carries geometry only. Select individual components/subsets or reduce distance
tolerance when widely separated islands make the input diagonal too permissive.

Exit codes:

- `0`: valid result and completed output/report.
- `1`: processing or I/O error.
- `2`: invalid argument count.
- `3`: diagnostic output has new self-intersections, changed component count or
  changed closedness; clients must not automatically apply it.

On success/exit 3, stdout and the optional report contain one
`CGAL_RESULT key=value ...` line: source/result counts, regions, UV seams and
residual, closedness, intersections, component counts, sampled error and duration.
Processing errors write `CGAL_FAIL message` to stderr and to an opened report.
`all_patches_remeshed=0` is not itself failure: some patches can remain unchanged
while other patches reduce. Clients must check exit status before reading output.

Error is estimated bidirectionally from vertices, edge midpoints and face
centroids. It is not a certified Hausdorff bound. Mesh checks do not guarantee
visual equivalence. The standalone CLI has no progress stream or built-in timeout;
clients control process cancellation/timeouts and clean temporary files.

Tests cover planar coverage and holes, input/overwrite protection, UV interpolation,
seams, material boundaries, mirrored and tiled UVs, non-affine detail, curved and
vertical surfaces, invalid input and the editor report protocol. Run from the
repository root with `ctest --test-dir build --output-on-failure`.

## Optional topology preparation

```sh
mbm-cgal-planar source.obj result.obj 10 .05 .000001 report.txt --repair-topology
```

`--repair-topology` uses the shared preparation routine: orient triangles and
split non-manifold connections while preserving positions and corner UVs/materials.
It does not remove degenerates or self-intersections. Reports include
`repair_enabled`, `repair_split_vertices` and `repair_reversed_faces`.
Topology metrics refer to the prepared source. Without the flag, input remains
strict. Both flags below require OBJ and all six positional arguments.

To consume the separate `mbm-cgal-repair` output, use `--preserve-topology`
instead of welding its separate vertex indices back together. This flag does not
skip mesh validation. Repair solves input topology, not UV/patch restrictions:
a valid run may leave the triangle count unchanged.
