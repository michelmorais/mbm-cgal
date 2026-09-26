# Windows build guide

This project produces four standalone command-line workers:

- `mbm-cgal-audit.exe`
- `mbm-cgal-planar.exe`
- `mbm-cgal-remesh.exe`
- `mbm-cgal-repair.exe`

They are built with CMake and MSVC. CGAL is not linked into the mini-mbm
engine.

## Prerequisites

Install:

1. Visual Studio 2022 with **Desktop development with C++**;
2. MSVC v143 and a Windows 10 or Windows 11 SDK;
3. CMake 3.25.1 or newer;
4. Python 3, for the test suite;
5. vcpkg;
6. CGAL 6.0 or newer, Eigen3, Boost, GMP and MPFR.

The Visual Studio developer prompt is recommended because it initializes the
MSVC environment, including `cl.exe`, `link.exe` and the Windows SDK paths.

## Install vcpkg

If PowerShell reports that `vcpkg` is not recognized, install it first. The
following commands place it in `C:\src\vcpkg`:

```powershell
New-Item -ItemType Directory -Force C:\src | Out-Null
git clone https://github.com/microsoft/vcpkg.git C:\src\vcpkg
& C:\src\vcpkg\bootstrap-vcpkg.bat
```

You can invoke the executable by its full path, without changing the global
`PATH`:

```powershell
& C:\src\vcpkg\vcpkg.exe version
```

Alternatively, add it only to the current PowerShell session:

```powershell
$env:Path += ";C:\src\vcpkg"
vcpkg version
```

## Install dependencies with vcpkg

From a developer prompt, run:

```powershell
& C:\src\vcpkg\vcpkg.exe install cgal eigen3 boost gmp mpfr --triplet x64-windows
```

If vcpkg resolves a CGAL version older than 6.0, update the vcpkg ports or
provide a separate CGAL 6.0+ installation and pass its CMake package location
through `CGAL_DIR`.

## First build: audit worker

Use a separate build directory for the Windows build. Replace the toolchain
path below with the absolute path to the local vcpkg installation:

```powershell
cmake -S . -B build-win `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DMBM_CGAL_BUILD_PLANAR=OFF `
  -DMBM_CGAL_BUILD_REMESH=OFF `
  -DMBM_CGAL_BUILD_AUDIT=ON `
  -DMBM_CGAL_BUILD_REPAIR=OFF

cmake --build build-win --config Release
ctest --test-dir build-win -C Release --output-on-failure
```

This validates the compiler, CMake package discovery, CGAL and the Python test
runner with the smallest target set.

## Build all workers

Once the audit worker succeeds, explicitly enable all four targets. CMake keeps
previous OFF values in its cache; omitting these options does not re-enable them:

```powershell
cmake -S . -B build-win `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DMBM_CGAL_BUILD_PLANAR=ON `
  -DMBM_CGAL_BUILD_REMESH=ON `
  -DMBM_CGAL_BUILD_AUDIT=ON `
  -DMBM_CGAL_BUILD_REPAIR=ON

cmake --build build-win --config Release
ctest --test-dir build-win -C Release --output-on-failure
cmake --install build-win --config Release --prefix install-win
```

The installed files are placed in:

```text
install-win/bin/
install-win/share/mbm-cgal/
```

On Windows, the install rules look for `gmp-10.dll` and `mpfr-6.dll` in the
selected vcpkg triplet (or dependency prefixes) and copy those found next to the
executables. Debug uses `debug/bin`; Release/RelWithDebInfo/MinSizeRel use `bin`.
Which DLLs are needed depends on the CGAL configuration and linkage; static
triplets may need neither. This is not recursive deployment of every dependency.
Use `dumpbin /DEPENDENTS install-win\bin\mbm-cgal-repair.exe` (and the other
workers) from the developer prompt, and verify the installed executables run
without relying on build-tree DLLs. If dependency versions use other DLL names,
collect the matching runtime files and update the install rule as appropriate.
The MSVC runtime (`MSVCP140.dll`, `VCRUNTIME140.dll` and the Windows CRT) is
provided by the Visual C++ Redistributable installed on the target machine.

The executables in the build tree are normally located at:

```text
build-win/tools/audit/Release/mbm-cgal-audit.exe
build-win/tools/planar/Release/mbm-cgal-planar.exe
build-win/tools/remesh/Release/mbm-cgal-remesh.exe
build-win/tools/repair/Release/mbm-cgal-repair.exe
```

## Troubleshooting

### `cl` or `link` is not recognized

Run CMake from the Visual Studio **x64 Native Tools Command Prompt**, or load
the Visual Studio developer environment before invoking CMake. A normal
PowerShell window may have CMake but not the MSVC tools on `PATH`.

### CGAL or Eigen3 cannot be found

Check that the vcpkg toolchain was passed during the first configuration. CMake
caches dependency locations, so remove the build directory and configure again
after changing the toolchain or dependency installation:

```powershell
Remove-Item -Recurse -Force build-win
```

Use a narrowly scoped build directory such as `build-win`; do not remove the
repository or the vcpkg installation.

If CMake reports that Eigen3 version `5.0.1` is incompatible with the requested
version `3.1`, use the current repository version. Its CMake files intentionally
do not request a specific Eigen major version because Eigen's package config can
reject an older requested major even when the required `Eigen3::Eigen` target is
available.

### Tests cannot start Python

Install Python 3 and ensure `python` or the Python launcher is available on
`PATH`. Alternatively configure with `-DBUILD_TESTING=OFF` to build without the
tests.

### Runtime DLL errors

A debug or release executable must use dependencies from the matching vcpkg
triplet/configuration. For distribution, collect the executable's dependent
DLLs and retain the corresponding third-party license and source obligations;
the repository's `THIRD_PARTY.md` records the relevant licensing scope.

## Integration with mini-mbm

In Mesh Debug, Image Mesh or Mesh3DGen, choose `install-win/bin` in
**Options > CGAL executable**. The table discovers all four `.exe` files and
shows descriptions/availability. **Refresh** updates the cached list. A green
entry confirms the file exists; it cannot guarantee the DLLs are present.
Changing tools does not require rebuilding mini-mbm.

## Validation scope

The current Windows changes have been reviewed for target flags, `.exe` discovery,
Release/Debug install paths and command syntax. A Linux CMake configure/install
test with synthetic GMP/MPFR files exercised both Windows install-rule branches. The Linux environment used for
this revision has no MSVC, Windows SDK, Wine or MinGW runtime; no native Windows
build or execution is claimed. Run the commands above and all CTest cases on
Windows before distributing Windows binaries. CTest checks the build-tree tools;
repeat representative OBJ commands with the installed tools to verify DLL deployment.
