# SPDX-License-Identifier: MIT
import pathlib
import subprocess
import sys
import tempfile
from collections import Counter

repair = sys.argv[1]
planar = sys.argv[2] if len(sys.argv) > 2 else None

def attributed_faces(path):
    points, uvs, result, material = [], [], [], ''
    for line in path.read_text().splitlines():
        parts = line.split()
        if not parts: continue
        if parts[0] == 'v': points.append(tuple(map(float, parts[1:])))
        elif parts[0] == 'vt': uvs.append(tuple(map(float, parts[1:])))
        elif parts[0] == 'usemtl': material = ' '.join(parts[1:])
        elif parts[0] == 'f':
            corners = []
            for corner in parts[1:]:
                vi, ti = map(int, corner.split('/'))
                corners.append((points[vi-1], uvs[ti-1]))
            result.append((material, tuple(sorted(corners))))
    return Counter(result)

with tempfile.TemporaryDirectory() as directory:
    root = pathlib.Path(directory)
    source = root/'source.obj'
    source.write_text('v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 -1 0\nv 0 0 1\n'
                      'vt 0 0\nvt 1 0\nvt 0 1\nvt 0 -1\nvt 0 0\n'
                      'usemtl surface\nf 1/1 2/2 3/3\nf 2/2 1/1 4/4\n'
                      'usemtl branch\nf 1/1 2/2 5/5\n')
    original=source.read_bytes()
    output=root/'repaired.obj'
    report=root/'report.txt'
    run=subprocess.run([repair,str(source),str(output),str(report)],capture_output=True,text=True)
    assert run.returncode==0,run.stderr
    assert source.read_bytes()==original
    assert attributed_faces(source)==attributed_faces(output), 'repair changed source positions, triangles, UVs or materials'
    fields=dict(item.split('=') for item in report.read_text().split()[1:])
    assert fields['source_triangles']==fields['result_triangles']=='3'
    assert int(fields['repair_split_vertices'])>0
    second=root/'second.obj'
    run=subprocess.run([repair,str(output),str(second),'--preserve-topology'],capture_output=True,text=True)
    assert run.returncode==0 and 'repair_split_vertices=0' in run.stdout,run.stderr
    assert attributed_faces(output)==attributed_faces(second)
    existing=output.read_bytes()
    run=subprocess.run([repair,str(source),str(output)],capture_output=True)
    assert run.returncode!=0 and output.read_bytes()==existing
    if planar:
        for name,mesh,flags in [('strict',source,[]),('prepare',source,['--repair-topology']),
                                ('preserve',output,['--preserve-topology'])]:
            result=root/(name+'.obj'); info=root/(name+'.txt')
            run=subprocess.run([planar,str(mesh),str(result),'10','.05','.000001',str(info),*flags],capture_output=True,text=True)
            if name=='strict': assert run.returncode==1 and not result.exists(),run.stderr
            else:
                assert run.returncode==0,run.stderr
                assert attributed_faces(mesh)==attributed_faces(result)
    bad=root/'bad.obj';bad.write_text('v 0 0 0\nv 1 0 0\nv 2 0 0\nvt 0 0\nf 1/1 2/1 3/1\n')
    run=subprocess.run([repair,str(bad),str(root/'bad-result.obj')],capture_output=True,text=True)
    assert run.returncode==1 and 'degenerate' in run.stderr
