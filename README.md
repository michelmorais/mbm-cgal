# MBM CGAL

Standalone geometry-processing tools using CGAL. Each tool is a separate command
line executable; mini-mbm and other clients exchange files with it. CGAL is not
linked into the mini-mbm engine.

## Tools

- [mbm-cgal-planar](tools/planar/README.md): planar/almost-planar mesh reduction,
  with OBJ corner UVs and material boundaries or geometry-only OFF input.
- [mbm-cgal-remesh](tools/remesh/README.md): isotropic remeshing of static
  triangular OBJ meshes, with UV/material chart and sharp-feature constraints.

- [mbm-cgal-repair](tools/repair/README.md): topology orientation and manifold splitting,
  preserving positions and triangle count.
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

### Windows

The supported Windows build uses MSVC, CMake and vcpkg. Install the **Desktop
development with C++** workload from Visual Studio, including MSVC v143 and a
Windows SDK. Then install vcpkg and the native dependencies from a vcpkg
enabled developer prompt. If `vcpkg` is not recognized, install it first:

```powershell
New-Item -ItemType Directory -Force C:\src | Out-Null
git clone https://github.com/microsoft/vcpkg.git C:\src\vcpkg
& C:\src\vcpkg\bootstrap-vcpkg.bat
& C:\src\vcpkg\vcpkg.exe install cgal eigen3 boost gmp mpfr --triplet x64-windows
```

CGAL 6.0 or newer is required. Check the installed version if vcpkg selects an
older port before configuring this repository. The first configuration can
build only the audit worker, which is a useful way to validate the toolchain:

```powershell
cmake -S . -B build-win `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DMBM_CGAL_BUILD_PLANAR=OFF `
  -DMBM_CGAL_BUILD_REMESH=OFF `
  -DMBM_CGAL_BUILD_AUDIT=ON `
  -DMBM_CGAL_BUILD_REPAIR=OFF

cmake --build build-win --config Release
ctest --test-dir build-win -C Release --output-on-failure
```

After the audit worker passes, configure and build all workers:

```powershell
cmake -S . -B build-win `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DMBM_CGAL_BUILD_PLANAR=ON `
  -DMBM_CGAL_BUILD_REMESH=ON `
  -DMBM_CGAL_BUILD_AUDIT=ON `
  -DMBM_CGAL_BUILD_REPAIR=ON

cmake --build build-win --config Release
ctest --test-dir build-win -C Release --output-on-failure
cmake --install build-win --config Release --prefix install-win
```

The resulting executables have the `.exe` suffix. With this generator, the
build-tree binaries are under `build-win/tools/<tool>/Release/`; installed
binaries are under `install-win/bin/`. The Python tests require Python 3 to be
available on `PATH`.

For a reproducible Windows setup and troubleshooting notes, see
[docs/windows-build.md](docs/windows-build.md).

For custom dependencies, supply `CGAL_DIR`, `CMAKE_PREFIX_PATH` and the appropriate
package include/library paths. Use `-DBUILD_TESTING=OFF` to build without Python.
Installed executables are placed in `install/bin/`; licenses are installed under
`install/share/mbm-cgal/`. Windows builds produce `.exe` files.

## Use from mini-mbm

In Mesh Debug, Image Mesh or Mesh3DGen, select the installed **folder** under
**Options > CGAL executable**. The shared table discovers `mbm-cgal-planar`,
`mbm-cgal-remesh`, `mbm-cgal-repair` and `mbm-cgal-audit` (with `.exe` on Windows),
shows a description and marks files found in green. **Refresh** rescans after
installing tools. Green means file presence, not successful execution or DLL validation.
The folder preference is `.mini-mbm-cgal-folder` in APPDATA/HOME, overridden by
`MBM_CGAL_FOLDER_CONFIG`; it takes priority over legacy individual paths.
Restart another open editor after changing the preference elsewhere.

The mini-mbm editors expose planar reduction as `cgal`; `cgal_qem` can follow
that pass with the engine's QEM when needed to meet a triangle budget. Isotropic
remeshing is a separate **Remesh** operation, not a reduction mode, and may
increase triangle count. Both workers run offline; CGAL is not linked into the
engine. The editors retain preview, cancellation, attribute transfer and
undo/history responsibilities. The CLI OBJ format has no physics data; editor
adapters copy authored collision shapes when rebuilding the MSH. Remesh requires
static triangle meshes and does not transfer skin weights or animation.

The editors expose edge length, iterations (10 by default, range 1..50) and
feature angle (14.5 by default). They do not expose a triangle-count target.
The CLI's optional `--target-triangles` heuristic remains available to direct
integrations and does not guarantee a count. Remesh prioritizes triangulation
regularity and can increase or decrease triangle count.

The Image Mesh Editor can run Remesh after optional simplification as a separate
stage. The mesh3dgen editor also uses the standalone worker for a local MSH
variant; this does not submit or replace its paid remote Remesh task.

Engine/editor integration tests live in the mini-mbm repository; this repository
contains standalone geometry, UV and protocol tests.

## Standalone topology repair

`mbm-cgal-repair` repairs triangle connectivity and orientation without remeshing.
See [repair usage and limitations](tools/repair/README.md). CGAL planar and Remesh
also accept `--repair-topology` using the same preparation routine.
The editors run the separate repair executable before processing when requested,
and preserve source indices both in OBJ export and with `--preserve-topology`
on every repair/planar/remesh invocation. Without that CLI flag, exact-position
welding can undo previously separated indices. Audit still welds exact positions,
so its connectivity metrics can differ from indexed repair/processing.

**Repair now** is independent of the preprocessing checkbox. A zero-split,
zero-reversal result is a no-op: Mesh Debug does not dirty the source or replace
undo; Mesh3DGen records an unchanged run without creating another variant.
Changed Mesh3DGen repairs create a separate MSH variant. Hole filling and
self-intersection removal are not provided.

OBJ workers do not transfer authored normals. Repair adapters carry normals by
source-face/corner correspondence. Remesh adapters reconstruct area-weighted
normals within connected fans using the feature-angle threshold, preserving
existing seams and sharp edges. Vertex budgets apply after this reconstruction.

## Future work

The following are proposed extensions or new tools; these capabilities are not
implemented yet:

- **Extend the existing mesh repair tool:** add hole filling and treatment of
  self-intersections to `mbm-cgal-repair`, guided by Audit diagnostics. The
  [existing repair tool](tools/repair/README.md) already corrects face orientation
  and splits non-manifold connections while preserving positions and triangle
  count. Planar and Remesh can run the same topology preparation through
  `--repair-topology`. Hole filling, self-intersection removal and removal of
  degenerate faces remain outside the implemented repair scope.
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
