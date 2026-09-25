# /*-----------------------------------------------------------------------------------------------------------------------|
# | MIT License (MIT)                                                                                                      |
# | Copyright (C) 2026 by Michel Braz de Morais <michel.braz.morais@gmail.com>                                             |
# |                                                                                                                        |
# | Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated           |
# | documentation files (the "Software"), to deal in the Software without restriction, including without limitation       |
# | the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and       |
# | to permit persons to whom the Software is furnished to do so, subject to the following conditions:                     |
# | The above copyright notice and this permission notice shall be included in all copies or substantial portions.         |
# | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   |
# | WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR  |
# | COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR       |
# | OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.       |
# |-----------------------------------------------------------------------------------------------------------------------*/
"""Standalone CGAL geometry regressions; requires a built mbm-cgal-planar."""
import pathlib
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1]).resolve()

def read_off(path):
    words = path.read_text().split()
    assert words[0] == 'OFF'
    nv, nf = int(words[1]), int(words[2])
    cursor = 4
    vertices = []
    for _ in range(nv):
        vertices.append(tuple(map(float, words[cursor:cursor+3])))
        cursor += 3
    triangles = []
    for _ in range(nf):
        assert words[cursor] == '3'
        triangles.append(tuple(map(int, words[cursor+1:cursor+4])))
        cursor += 4
    return vertices, triangles

def area(vertices, triangles):
    total = 0
    for ia, ib, ic in triangles:
        a, b, c = vertices[ia], vertices[ib], vertices[ic]
        total += abs((b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]))/2
    return total

with tempfile.TemporaryDirectory(prefix='mbm-cgal-test-') as folder:
    root = pathlib.Path(folder)
    for hole in (False, True):
        source, output = root / f'{hole}.off', root / f'{hole}-out.off'
        vertices = [(x, y, 0) for y in range(9) for x in range(9)]
        triangles = []
        for y in range(8):
            for x in range(8):
                if hole and 3 <= x <= 4 and 3 <= y <= 4:
                    continue
                a = y*9+x
                triangles.extend([(a, a+1, a+10), (a, a+10, a+9)])
        source.write_text(f'OFF\n{len(vertices)} {len(triangles)} 0\n' +
                          ''.join(f'{x} {y} {z}\n' for x, y, z in vertices) +
                          ''.join(f'3 {a} {b} {c}\n' for a, b, c in triangles))
        before = source.read_bytes()
        result = subprocess.run([str(binary), str(source), str(output), '5', '.01'],
                                capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stdout + result.stderr
        assert 'result_self_intersections=0' in result.stdout
        rv, rt = read_off(output)
        assert len(rt) == (8 if hole else 2), result.stdout
        assert abs(area(rv, rt)-area(vertices, triangles)) < 1e-9, 'hole/coverage changed'
        assert source.read_bytes() == before
        saved = output.read_bytes()
        overwrite = subprocess.run([str(binary), str(source), str(output), '5', '.01'],
                                   capture_output=True, text=True, timeout=30)
        assert overwrite.returncode != 0 and output.read_bytes() == saved
    bad = subprocess.run([str(binary), str(source), str(root/'invalid.off'), 'nan', '.01'],
                         capture_output=True, text=True, timeout=30)
    assert bad.returncode != 0 and not (root/'invalid.off').exists()
print('CGAL_PLANAR_TEST_OK flat / hole / coverage / input preserved / overwrite refused / invalid input')
