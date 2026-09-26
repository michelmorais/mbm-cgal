/*-----------------------------------------------------------------------------------------------------------------------|
| MIT License (MIT)                                                                                                      |
| Copyright (C) 2026 by Michel Braz de Morais <michel.braz.morais@gmail.com>                                             |
|                                                                                                                        |
| Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated           |
| documentation files (the "Software"), to deal in the Software without restriction, including without limitation       |
| the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and       |
| to permit persons to whom the Software is furnished to do so, subject to the following conditions:                     |
| The above copyright notice and this permission notice shall be included in all copies or substantial portions.         |
| THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   |
| WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR  |
| COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR       |
| OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.       |
|-----------------------------------------------------------------------------------------------------------------------*/


#include "../common/repair-input.h"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 3 || argc > 5)
    {
        std::cerr << "Usage: mbm-cgal-repair input.obj output.obj [report-path] [--preserve-topology]\n";
        return 2;
    }
    std::ofstream report;
    try
    {
        const bool preserveTopology = argc >= 4 && std::string(argv[argc-1]) == "--preserve-topology";
        const int positionalCount = argc - (preserveTopology ? 1 : 0);
        if (positionalCount > 4) throw std::runtime_error("unexpected arguments");
        const std::filesystem::path output(argv[2]);
        const auto temporary = output.string() + ".tmp";
        if (std::filesystem::path(argv[1]).extension() != ".obj" || output.extension() != ".obj")
            throw std::runtime_error("input and output must be .obj files");
        if (std::filesystem::exists(output) || std::filesystem::exists(temporary))
            throw std::runtime_error("output already exists; choose a new file");
        if (positionalCount == 4)
        {
            if (std::filesystem::exists(argv[3]) || std::filesystem::path(argv[3]) == output ||
                std::filesystem::path(argv[3]) == temporary)
                throw std::runtime_error("report already exists or aliases output");
            report.open(argv[3]);
            if (!report) throw std::runtime_error("cannot open report");
        }
        mbm_cgal_uv::MESH mesh;
        auto input = mbm_cgal_uv::read(argv[1], mesh, true, !preserveTopology);
        const auto sourceVertices = mesh.number_of_vertices();
        const auto repaired = mbm_cgal_uv::repair(input, mesh);
        if (mesh.number_of_faces() == 0 || !CGAL::is_valid_polygon_mesh(mesh) || !CGAL::is_triangle_mesh(mesh))
            throw std::runtime_error("repair did not produce a valid triangle mesh");
        std::ofstream file(temporary);
        if (!file) throw std::runtime_error("cannot open output");
        file << std::setprecision(17) << "# MBM topology repair; preserve OBJ vertex indices when importing\n";
        for (const auto &library : input.libraries) file << "mtllib " << library << '\n';
        for (auto vertex : mesh.vertices())
        {
            const auto &p = mesh.point(vertex);
            file << "v " << p.x() << ' ' << p.y() << ' ' << p.z() << '\n';
        }
        std::size_t uvIndex = 1;
        for (const auto &face : input.faces)
        {
            file << "usemtl " << face.material << '\n';
            for (const auto &uv : face.uv) file << "vt " << uv[0] << ' ' << uv[1] << '\n';
            file << 'f';
            for (unsigned k = 0; k < 3; ++k) file << ' ' << face.vertices[k].idx()+1 << '/' << uvIndex++;
            file << '\n';
        }
        file.close();
        if (!file) throw std::runtime_error("cannot write output");
        std::filesystem::rename(temporary, output);
        std::ostringstream line;
        line << "CGAL_REPAIR_RESULT source_vertices=" << sourceVertices
             << " source_triangles=" << input.faces.size()
             << " result_vertices=" << mesh.number_of_vertices()
             << " result_triangles=" << mesh.number_of_faces()
             << " repair_enabled=1 repair_split_vertices=" << repaired.splitVertices
             << " repair_reversed_faces=" << repaired.reversedFaces << '\n';
        std::cout << line.str();
        if (report.is_open() && !(report << line.str())) throw std::runtime_error("cannot write report");
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "CGAL_FAIL " << error.what() << '\n';
        if (report.is_open()) report << "CGAL_FAIL " << error.what() << '\n';
        return 1;
    }
}
