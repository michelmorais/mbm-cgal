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
"""UV/material regressions for the external OBJ path, independent of the engine."""
import pathlib
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1]).resolve()

def parse(path):
    positions, uv, faces = [], [], []
    material = ''
    for line in path.read_text().splitlines():
        parts = line.split()
        if not parts:
            continue
        if parts[0] == 'v': positions.append(tuple(map(float, parts[1:])))
        if parts[0] == 'vt': uv.append(tuple(map(float, parts[1:])))
        if parts[0] == 'usemtl': material = ' '.join(parts[1:])
        if parts[0] == 'f':
            pairs = [tuple(int(v)-1 for v in corner.split('/')) for corner in parts[1:]]
            faces.append((material, [(positions[v], uv[t]) for v, t in pairs]))
    return faces

def sample(faces, x, y, axes=(0,1)):
    for material, face in faces:
        a, b, c = [(p[axes[0]], p[axes[1]]) for p, uv in face]
        det = (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
        w1 = ((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det
        w2 = ((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
        if min(1-w1-w2, w1, w2) >= -1e-10:
            result = tuple((1-w1-w2)*face[0][1][i]+w1*face[1][1][i]+w2*face[2][1][i] for i in range(2))
            return material, result
    raise AssertionError(('coverage gap', x, y))

with tempfile.TemporaryDirectory(prefix='mbm-cgal-uv-test-') as directory:
    root = pathlib.Path(directory)
    # Continuous affine field, discontinuous seam, materials, mirrored chart,
    # non-affine interior UV detail, tiled UVs outside [0,1], and a tiny seam
    # below the fitting tolerance (it must still survive as a discontinuity).
    for kind in ('affine', 'seam', 'materials', 'mirrored', 'nonaffine', 'tiled', 'tiny_seam', 'curved', 'vertical'):
        lines = ['mtllib textures.mtl']
        for y in range(9):
            for x in range(9):
                point = (0,x,y) if kind == 'vertical' else (x,y,.01*x*x if kind == 'curved' else 0)
                lines.append('v ' + ' '.join(map(str,point)))
        uv_count = 0
        for y in range(8):
            for x in range(8):
                a = y*9+x+1
                material = 'right' if kind == 'materials' and x >= 4 else 'left'
                lines.append('usemtl ' + material)
                for tri in ((a, a+1, a+10), (a, a+10, a+9)):
                    corners = []
                    for v in tri:
                        px, py = (v-1)%9, (v-1)//9
                        u, vv = px/8, py/8
                        if kind in ('seam', 'tiny_seam') and x >= 4: u += 1 if kind == 'seam' else 1e-8
                        if kind == 'mirrored' and x >= 4: u = 1-u
                        if kind == 'nonaffine' and px == 4 and py == 4: u += .2
                        if kind == 'tiled': u = u*3-1; vv = vv*4-2
                        uv_count += 1
                        lines.append(f'vt {u:.17g} {vv:.17g}')
                        corners.append(f'{v}/{uv_count}')
                    lines.append('f ' + ' '.join(corners))
        source, output = root/(kind+'.obj'), root/(kind+'-out.obj')
        source.write_text('\n'.join(lines)+'\n')
        original = source.read_bytes()
        result = subprocess.run([str(binary), str(source), str(output), '60' if kind == 'curved' else '5', '.1' if kind == 'curved' else '.01'], capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stdout+result.stderr
        before, after = parse(source), parse(output)
        assert len(after) < len(before), (kind, result.stdout)
        if kind in ('affine', 'tiled', 'vertical'): assert len(after) == 2
        # Check interpolation throughout every grid cell, not just retained UVs.
        for iy in range(32):
            for ix in range(32):
                x, y = (ix+.37)/4, (iy+.61)/4
                axes=(1,2) if kind == 'vertical' else (0,1)
                ma, ua = sample(before, x, y, axes); mb, ub = sample(after, x, y, axes)
                assert ma == mb and max(abs(a-b) for a, b in zip(ua, ub)) < 1e-9, (kind, x, y, ua, ub)
        assert source.read_bytes() == original
        assert f'mtllib {root / "textures.mtl"}' in output.read_text()
        # Every output corner must keep one of that material's original UVs at that position.
        originals = {(m, p, uv) for m, f in before for p, uv in f}
        assert all((m, p, uv) in originals for m, f in after for p, uv in f)
        print(f'CGAL_UV_CASE_OK {kind}: {len(before)} -> {len(after)}')
    negative = root/'negative.obj'
    negative.write_text('v 0 0 0\nv 1 0 0\nv 0 1 0\nvt .13 .17\nvt .91 .22\nvt .06 .85\nvn 0 0 1\nf -3/-3/-1 -2/-2/-1 -1/-1/-1\n')
    result = subprocess.run([str(binary), str(negative), str(root/'negative-out.obj'), '5', '.01', '0'],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
    negative_faces = parse(root/'negative-out.obj')
    assert len(negative_faces) == 1
    assert {uv for _, f in negative_faces for _, uv in f} == {(.13,.17),(.91,.22),(.06,.85)}
    invalid = root/'missing-uv.obj'
    invalid.write_text('v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n')
    failed = subprocess.run([str(binary), str(invalid), str(root/'bad.obj'), '5', '.01'], capture_output=True, text=True, timeout=30)
    assert failed.returncode != 0 and not (root/'bad.obj').exists()
    report = root/'editor.report'
    result = subprocess.run([str(binary), str(negative), str(root/'editor-out.obj'), '5', '.01', '0.000001', str(report)], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0 and report.read_text().startswith('CGAL_RESULT ')
    report_bytes = report.read_bytes()
    refused = subprocess.run([str(binary), str(negative), str(root/'refused.obj'), '5', '.01', '0.000001', str(report)], capture_output=True, text=True, timeout=30)
    assert refused.returncode != 0 and report.read_bytes() == report_bytes and not (root/'refused.obj').exists()
    failure_report = root/'failure.report'
    failed = subprocess.run([str(binary), str(invalid), str(root/'failure.obj'), '5', '.01', '0.000001', str(failure_report)], capture_output=True, text=True, timeout=30)
    assert failed.returncode != 0 and failure_report.read_text().startswith('CGAL_FAIL ') and not (root/'failure.obj').exists()
    print('CGAL_EDITOR_PROTOCOL_OK report / refusal to overwrite / failure diagnostic')
print('CGAL_UV_TEST_OK interpolation / seams / materials / mirrored / nonaffine / tiled / tiny seam / curved / vertical / invalid input')
