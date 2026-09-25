# MBM CGAL

Standalone geometry-processing tools using CGAL. Each tool is a separate command
line executable; mini-mbm and other clients exchange files with it. CGAL is not
linked into the mini-mbm engine.

## Tools

- [mbm-cgal-planar](tools/planar/README.md): planar/almost-planar mesh reduction,
  with OBJ corner UVs and material boundaries or geometry-only OFF input.

Additional tools belong in `tools/<name>/` with their own targets and tests.

## Build and test

Requires C++17, CMake >= 3.25.1, CGAL >= 6.0, Eigen3, Boost and the arithmetic
libraries selected by CGAL (GMP/MPFR in the tested Linux build). Python 3 is used
for tests. Dependencies are not downloaded automatically.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$PWD/install"
```

For custom dependencies, supply `CGAL_DIR`, `CMAKE_PREFIX_PATH` and the appropriate
package include/library paths. Use `-DBUILD_TESTING=OFF` to build without Python.
Installed executables are placed in `install/bin/`; licenses are installed under
`install/share/mbm-cgal/`. Windows builds produce `.exe` files.

## Use from mini-mbm

In Mesh Debug or Image Mesh Editor, open **Options > CGAL executable**, choose the
full path to `mbm-cgal-planar` (or `mbm-cgal-planar.exe`) and **Save path**.
Select **Coplanar (CGAL external)** in general simplification. The executable's
location can change without rebuilding the engine. The editors retain their
preview, cancellation, UV/material transfer and undo/history responsibilities.
Engine/editor integration tests live in the mini-mbm repository; this repository
contains standalone geometry, UV and protocol tests.

## License

GPL-3.0-or-later for the combined tools. Migrated wrapper files retain their MIT
notices. See [NOTICE.md](NOTICE.md), [LICENSE](LICENSE) and
[THIRD_PARTY.md](THIRD_PARTY.md) for scope, attribution and binary distribution.
