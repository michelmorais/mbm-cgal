# MBM CGAL

Standalone geometry-processing tools using CGAL. Each tool is a separate command
line executable; mini-mbm and other clients exchange files with it. CGAL is not
linked into the mini-mbm engine.

## Tools

- [mbm-cgal-planar](tools/planar/README.md): planar/almost-planar mesh reduction,
  with OBJ corner UVs and material boundaries or geometry-only OFF input.
- [mbm-cgal-remesh](tools/remesh/README.md): isotropic remeshing of static
  triangular OBJ meshes, with UV/material chart and sharp-feature constraints.

- [mbm-cgal-audit](tools/audit/README.md): read-only OBJ/OFF geometry diagnostics
  with a versioned JSON API, also exposed through Lua and the three editors.

Additional tools belong in `tools/<name>/` with their own targets and tests.

## Build and test

Requires C++17, CMake >= 3.25.1, CGAL >= 6.0, Eigen3, Boost and the arithmetic
libraries selected by CGAL (GMP/MPFR in the tested Linux build). Python 3 is used
for tests. Dependencies are not downloaded automatically.

On Debian 12, the distribution package is CGAL 5.5 and does not satisfy the
CGAL >= 6.0 requirement. Install the build dependencies and CGAL 6.0 system-wide:

```sh
sudo apt install build-essential libboost-dev libeigen3-dev libgmp-dev libmpfr-dev
wget -O /tmp/CGAL-6.0.tar.xz \
    https://github.com/CGAL/cgal/releases/download/v6.0/CGAL-6.0.tar.xz
mkdir -p "$HOME/src"
tar -xf /tmp/CGAL-6.0.tar.xz -C "$HOME/src"
cmake -S "$HOME/src/CGAL-6.0" -B /tmp/cgal-6.0-build \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build /tmp/cgal-6.0-build -j2
sudo cmake --install /tmp/cgal-6.0-build
```

After a successful install, the source archive and build directory are no
longer needed and can be removed. Keep `/usr/local`, which contains the
system-wide CGAL installation:

```sh
rm -rf "$HOME/src/CGAL-6.0" /tmp/cgal-6.0-build /tmp/CGAL-6.0.tar.xz
```

Then configure this repository by pointing CMake at the installed package:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCGAL_DIR=/usr/local/lib/CGAL
cmake --build build -j2
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$PWD/install"
```

For custom dependencies, supply `CGAL_DIR`, `CMAKE_PREFIX_PATH` and the appropriate
package include/library paths. Use `-DBUILD_TESTING=OFF` to build without Python.
Installed executables are placed in `install/bin/`; licenses are installed under
`install/share/mbm-cgal/`. Windows builds produce `.exe` files.

## Use from mini-mbm

In Mesh Debug or Image Mesh Editor, configure the full paths to
`mbm-cgal-planar`, `mbm-cgal-remesh` and `mbm-cgal-audit` (or their `.exe` files) under
**Options > CGAL executable** and save them. Their locations can change without
rebuilding the engine.

The mini-mbm editors expose planar reduction as `cgal`; `cgal_qem` can follow
that pass with the engine's QEM when needed to meet a triangle budget. Isotropic
remeshing is a separate **Remesh** operation, not a reduction mode, and may
increase triangle count. Both workers run offline; CGAL is not linked into the
engine. The editors retain preview, cancellation, attribute transfer and
undo/history responsibilities. The CLI OBJ format has no physics data; editor
adapters copy authored collision shapes when rebuilding the MSH. Remesh requires
static triangle meshes and does not transfer skin weights or animation.

The Image Mesh Editor can run Remesh after optional simplification as a separate
stage. The mesh3dgen editor also uses the standalone worker for a local MSH
variant; this does not submit or replace its paid remote Remesh task.

Engine/editor integration tests live in the mini-mbm repository; this repository
contains standalone geometry, UV and protocol tests.

## Future work

The following are proposals, not implemented tools or engine features:

- **Mesh repair:** build on the read-only Audit diagnostics as a later, explicit
  operation that writes a separate result.
- **Collision proxies:** evaluate offline convex-hull or convex-decomposition
  generation for Bullet, with editor preview and an asset-format path. The
  expected benefit is cheaper collision geometry; prioritize this if 3D mesh
  collision is an active engine use case.
- **Image-mesh triangulation:** benchmark a CGAL constrained-triangulation
  backend against the engine's existing constrained and relief-aware
  triangulation. Adopt it only if representative assets show a measurable
  quality or performance improvement.

The engine's QEM simplifier already handles engine-specific attributes and
deformation data, so replacing it with a generic CGAL simplifier is not a goal.
Keep CGAL as an optional offline/editor tool; assess licensing before bundling
its executable with engine distributions.

## License

GPL-3.0-or-later for the combined tools. Migrated wrapper files retain their MIT
notices. See [NOTICE.md](NOTICE.md), [LICENSE](LICENSE) and
[THIRD_PARTY.md](THIRD_PARTY.md) for scope, attribution and binary distribution.
