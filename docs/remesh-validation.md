# Remesh validation — 2026-09-25

The pending implementation was reviewed across mbm-cgal, mini-mbm and mesh3dgen.
Changes remain uncommitted. No paid provider API was called.

## Corrections made during validation

- Accept `CGAL_REMESH_RESULT` in the mini-mbm adapter, independently of the planar protocol.
- Use the actual single-value ImGui Checkbox return contract for Mesh Debug's Remesh toggle.
- Balance the mesh3dgen simplification UI's disabled scopes and prevent concurrent local operations.
- Avoid passing an empty collision-shape array to `setPhysics`; preserve authored shapes.
- Validate material subsets and the 65535-vertex output limit in the mesh3dgen adapter.
- Cancel and clean up the external worker when the mesh3dgen scene ends.
- Reuse mesh3dgen's engine-root discovery when locating the OBJ adapter.
- Keep the main Lua chunk under the repository's local-variable test limit.
- Reject empty/degenerate input and allow UV transfer on small-scale valid geometry.
- Explain topology rejection in worker reports and editor errors.
- Label fractional edge-length controls correctly; `.03` means 3% of the diagonal.

## Passed checks

| Check | Result |
|---|---|
| Release CMake build and local install | Both tools built; Remesh installed in `install/bin/` |
| mbm-cgal CTest | 3/3 passed |
| Remesh geometry regression | Refinement, material boundaries, UV seams, finite UVs, topology, invalid arguments, overwrite refusal, empty/degenerate input and small-scale geometry |
| mini-mbm model tests | Settings, persistence/migration, method combinations and Remesh checkbox toggle/idle behavior |
| Mesh Debug, real engine | Apply, restore, cancel, triangle-count increase, UV/material/physics preservation and error paths |
| Image Mesh Editor, real engine | Planar/QEM regressions, Remesh rejection and successful retry, preview/export and idle cache behavior |
| mesh3dgen adapter, real engine | MSH round trip, UVs, material roles, collision shapes and refinement |
| mesh3dgen focused Python suite | 24 passed: simplification, local/remote Remesh and Lua local-variable limit |
| mesh3dgen Lua regressions | Local mesh controls, asynchronous Remesh lifecycle and simplification bridge passed |
| mesh3dgen Lua syntax | 29 application files parsed successfully |
| mesh3dgen application smoke | Started and exited with an isolated empty temporary store |
| Whitespace checks | `git diff --check` passed in all three repositories |

Engine checks used the existing Linux debug binary and the modified Lua modules.
The engine version header was advanced to 7.300.0; the engine C++ binary was not
rebuilt because the runtime implementation did not change. Interactive mouse
click/drag behavior was not manually tested; controls were exercised with Lua
regressions and the real editors were driven by test scenes.

The authored Image Mesh fixture requires `MBM_CGAL_PROJECT` (the local run used
`/tmp/cgal-integrated.imesh`). At edge fraction `.05`, two iterations and feature
angle `45`, it develops new self-intersections and is correctly rejected. The
same fixture succeeds with feature angle `0`, constraining all source edges.
Arbitrary settings are not guaranteed to produce an intersection-free result.

## Existing mesh3dgen suite failures

The initial full run executed 983 Python tests and reported 8 failures and 7
errors. The new local-variable-limit failure was fixed and explicitly retested.
A clean export of mesh3dgen HEAD `5d4e3eb` independently reproduced the other
7 failures and 7 errors (979 tests). These concern presentation assertions,
provider descriptors, the asset-context catalog, and image/video/mesh export
layout expectations. They are outside the Remesh implementation.

The Lua sweep initially passed 32 of 33 existing test files. The remaining
`operation_model_selection_ui_test.lua` fails because its mock lacks
`model_information_tooltip`; the same failure was reproduced at HEAD `5d4e3eb`.
The two newly added Lua regression files also pass.

The final full Python suite was not rerun after the focused corrections; the
24 relevant Python cases, modified Lua paths and affected engine smoke tests
were rerun successfully.
