# Copyright and licensing

Copyright (c) 2026 Michel Braz de Morais <michel.braz.morais@gmail.com>.

MBM CGAL tools are distributed under **GNU GPL version 3 or, at your option,
any later version** (`GPL-3.0-or-later`). See [LICENSE](LICENSE) for the complete
GNU GPL version 3 text. New project files use that license unless marked otherwise.

The planar wrapper sources, CMake file and Python tests migrated from mini-mbm
retain their original MIT license notices. Their MIT permission remains available
for those files individually; see [LICENSES/MIT.txt](LICENSES/MIT.txt). Combining
the wrapper with the GPL parts of CGAL does not make the resulting executable
MIT-only. Preserve both the original notices and the applicable GPL terms when
distributing the combined tool.

CGAL is an external dependency and is not vendored here. Its planar remeshing,
region growing and related package headers carry their own copyrights, including
GeometryFactory and other CGAL contributors, and their respective licenses.
The planar remeshing header in the tested CGAL 6.0 release identifies
Copyright (c) 2018-2023 GeometryFactory (France), author Sébastien Loriot,
and `GPL-3.0-or-later OR LicenseRef-Commercial`.
This project uses the open-source licensing route; no commercial CGAL license
is granted by this repository. See [THIRD_PARTY.md](THIRD_PARTY.md).

For binary releases, include license notices and provide the complete
Corresponding Source and build/install information required by the GPL,
including applicable dependency sources. A link to this repository alone is
not a substitute for packaging the required Corresponding Source of a release.
System-library exceptions and dynamic-library obligations depend on the actual
release contents; inspect the dependencies of each distributed artifact.
