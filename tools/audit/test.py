#!/usr/bin/env python3
"""Read-only audit contract and geometric regression fixtures."""
import hashlib
import json
import math
import pathlib
import subprocess
import sys
import tempfile

EXE = str(pathlib.Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix='mbm-audit-') as tmp:
    root = pathlib.Path(tmp)
    def run(name, content, *args):
        source = root / (name + '.obj')
        source.write_text(content)
        before = hashlib.sha256(source.read_bytes()).digest()
        result = subprocess.run([EXE, str(source), *args], capture_output=True, text=True)
        assert hashlib.sha256(source.read_bytes()).digest() == before
        assert result.returncode == 0, result.stderr
        report = json.loads(result.stdout)
        assert report['schema_version'] == 1 and report['status'] == 'completed'
        return report
    square = 'v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3\nf 1 3 4\n'
    output = root / 'audit.json'
    r = run('square', square, '--report', str(output))
    assert json.loads(output.read_text()) == r
    assert r['surface_area'] == 1 and r['triangles'] == 2
    assert r['boundary_edges'] == 4 and r['components'] == 1 and not r['closed']
    assert r['valid_polygon_mesh'] and r['has_self_intersections'] is False
    assert math.isclose(r['minimum_angle_degrees'], 45)
    assert math.isclose(r['triangle_quality_mean'], math.sqrt(3)/2)
    assert math.isclose(r['diagonal'], math.sqrt(2))
    original = output.read_bytes()
    result = subprocess.run([EXE, str(root/'square.obj'), '--report', str(output)], capture_output=True)
    assert result.returncode == 1 and output.read_bytes() == original
    r = run('skip', square, '--skip-self-intersections')
    assert r['self_intersections_status'] == 'not_requested' and r['has_self_intersections'] is None
    r = run('degenerate', 'v 0 0 0\nv 1 0 0\nv 2 0 0\nf 1 2 3\n')
    assert r['degenerate_triangles'] == 1 and r['triangle_quality_min'] == 0
    assert r['self_intersections_status'] == 'skipped_degenerate_triangles' and r['has_self_intersections'] is None
    r = run('welded', square.replace('f 1 3 4', 'v 0 0 0\nf 5 3 4'))
    assert r['duplicate_positions'] == 1 and r['boundary_edges'] == 4
    r = run('duplicate', square + 'f 1 2 3\n')
    assert r['duplicate_faces'] == 1 and not r['valid_polygon_mesh']
    assert r['non_manifold_edges'] == 1 and r['self_intersections_status'] == 'skipped_invalid_topology'
    r = run('orientation', square.replace('f 1 3 4', 'f 1 4 3'))
    assert r['orientation_conflicts'] == 1 and not r['valid_polygon_mesh']
    r = run('quad', square.replace('f 1 2 3\nf 1 3 4', 'f 1 2 3 4'))
    assert r['non_triangle_faces'] == 1 and r['surface_area'] is None
    assert r['self_intersections_status'] == 'skipped_non_triangles'
    r = run('crossing', 'v -1 0 0\nv 1 0 0\nv 0 1 0\nv 0 .2 -1\nv 0 .2 1\nv 0 -.5 0\nf 1 2 3\nf 4 5 6\n')
    assert r['components'] == 2 and r['has_self_intersections'] is True
    r = run('bowtie', 'v 0 0 0\nv 1 0 0\nv 0 1 0\nv -1 0 0\nv 0 -1 0\nf 1 2 3\nf 1 4 5\n')
    assert not r['valid_polygon_mesh'] and r['non_manifold_edges'] == 0
    r = run('tetra', 'v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\nf 1 3 2\nf 1 2 4\nf 2 3 4\nf 3 1 4\n')
    assert r['closed'] and r['components'] == 1 and not r['has_self_intersections']
    off = root/'square.off'
    off.write_text('OFF\n4 2 0\n0 0 0\n1 0 0\n1 1 0\n0 1 0\n3 0 1 2\n3 0 2 3\n')
    result = subprocess.run([EXE,str(off)],capture_output=True,text=True)
    assert result.returncode == 0 and json.loads(result.stdout)['surface_area'] == 1
    bad = root/'bad.obj'; bad.write_text('v 0 0 0\nf 1 2 99\n')
    result = subprocess.run([EXE,str(bad)],capture_output=True,text=True)
    assert result.returncode == 1 and json.loads(result.stderr)['status'] == 'failed'
print('Mesh audit geometry / JSON / read-only contract OK')
