# Standalone manifold topology repair

```sh
mbm-cgal-repair source.obj repaired.obj [report.txt] [--preserve-topology]
```

The worker shares its implementation with planar and Remesh preparation. It only
orients faces and splits non-manifold connections; it does not remesh, simplify,
move positions, delete triangles, fill holes, or remove self-intersections.
Input must be attributed triangular OBJ with UVs on every corner. Materials and
UVs are preserved; authored normals are not transferred. Degenerate faces fail.
MSH input is not supported. Source and existing output files are never overwritten.

The result line starts with `CGAL_REPAIR_RESULT` and reports source/result counts,
`repair_enabled=1`, `repair_split_vertices`, and `repair_reversed_faces`.
Exit 0 means success; 1 means validation/processing failure; 2 means wrong arity.
A manifold combinatorial structure is not a guarantee of a closed, watertight or
intersection-free surface. Splits may open seams or separate components.

By default exact positions are welded before repair. `--preserve-topology` skips
that welding for already prepared data. Downstream planar/Remesh must use
`--preserve-topology` to keep separate OBJ indices, otherwise their default weld
can undo the repair. Their topology validation remains active.

```sh
mbm-cgal-planar repaired.obj simplified.obj 10 .05 .000001 result.txt --preserve-topology
```
