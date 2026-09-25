# External dependencies

No third-party implementation source or binary is bundled in this repository.
CMake locates dependencies supplied by the build environment. License copies
are provided for reference and binary packaging; retain upstream notices when
packaging the libraries themselves.

| Dependency | Use / license |
|---|---|
| CGAL >= 6.0 | Planar remeshing, region growing, mesh validation, distance queries. Relevant higher-level packages use GPL-3.0-or-later or commercial terms; kernel/support parts may use LGPL-3.0-or-later. Check each included header/package. |
| Eigen3 >= 3.1 | Linear algebra; MPL-2.0 for the core used here. Optional modules can have other licenses; do not infer coverage for unused/unsupported modules. |
| Boost | Templates/utilities; Boost Software License 1.0. |
| GMP (tested configuration) | Multiprecision arithmetic; current releases offer LGPL-3.0-or-later or GPL-2.0-or-later. Check the selected version. |
| MPFR (tested configuration) | Floating-point arithmetic; LGPL-3.0-or-later for current releases. Check the selected version. |
| Python 3 | Runs tests; not part of the compiled executable. |

Copies: [GPL](LICENSES/GPL-3.0.txt), [LGPL](LICENSES/LGPL-3.0.txt),
[MPL](LICENSES/MPL-2.0.txt), [Boost](LICENSES/BSL-1.0.txt),
[CGAL distribution notice](LICENSES/CGAL-NOTICE.txt).

Authoritative references:

- https://www.cgal.org/license.html
- https://github.com/CGAL/cgal/blob/v6.0/Polygon_mesh_processing/include/CGAL/Polygon_mesh_processing/remesh_planar_patches.h
- https://libeigen.gitlab.io/eigen/docs-nightly/TopicPreprocessorDirectives.html (EIGEN_MPL2_ONLY)
- https://www.boost.org/LICENSE_1_0.txt
- https://gmplib.org/manual/Copying
- https://www.mpfr.org/mpfr-current/mpfr.html#Copying

The runtime dependency set varies by platform and build configuration. GMP/MPFR
are externally linked in the tested Linux build. Before publishing a binary,
record exact dependency versions, preserve their notices, and fulfill the
source/relinking requirements applicable to the actual release.
