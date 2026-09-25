#!/usr/bin/env python3
import math
import pathlib
import subprocess
import sys
import tempfile


def make_grid(path, charts=False):
    lines = ["mtllib source.mtl", "usemtl surface"]
    for y in range(5):
        for x in range(5):
            lines.append(f"v {x / 4} {y / 4} 0")
    for y in range(5):
        for x in range(5):
            lines.append(f"vt {x / 4} {y / 4}")
    if charts:
        for y in range(5):
            for x in range(5):
                lines.append(f"vt {10 + x / 4} {y / 4}")
    for y in range(4):
        for x in range(4):
            a = y * 5 + x + 1
            b, c, d = a + 1, a + 5, a + 6
            offset = 25 if charts and x >= 2 else 0
            if charts:
                lines.append("usemtl right" if offset else "usemtl surface")
            lines.append(f"f {a}/{a+offset} {b}/{b+offset} {d}/{d+offset}")
            lines.append(f"f {a}/{a+offset} {d}/{d+offset} {c}/{c+offset}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    path.with_suffix(".mtl").write_text("newmtl surface\nKd 1 1 1\n", encoding="utf-8")


def main():
    executable = pathlib.Path(sys.argv[1])
    with tempfile.TemporaryDirectory(prefix="mbm-cgal-remesh-") as temp:
        root = pathlib.Path(temp)
        source, output, report = root / "source.obj", root / "output.obj", root / "report.txt"
        make_grid(source)
        completed = subprocess.run([str(executable), str(source), str(output), "0.2", "2", "45", str(report)],
                                   text=True, capture_output=True, check=False)
        assert completed.returncode == 0, completed.stderr
        line = report.read_text(encoding="utf-8").strip()
        assert line.startswith("CGAL_REMESH_RESULT "), line
        assert "source_triangles=32" in line, line
        assert "sampled_error_fraction=" in line, line
        result = output.read_text(encoding="utf-8")
        assert "mtllib " in result and "usemtl surface" in result
        assert "vt " in result and "f " in result

        # Exercise refinement (including newly created faces), chart propagation,
        # UV seams, material preservation and open-boundary topology.
        make_grid(source, charts=True)
        refined, refined_report = root / "refined.obj", root / "refined.txt"
        run = subprocess.run([str(executable), str(source), str(refined), "0.06", "3", "45", str(refined_report)],
                             text=True, capture_output=True)
        assert run.returncode == 0, run.stderr
        fields = dict(part.split("=") for part in refined_report.read_text().split()[1:])
        assert int(fields["result_triangles"]) > 32, fields
        assert int(fields["charts"]) == 2, fields
        assert fields["source_components"] == fields["result_components"] == "1"
        assert fields["source_closed"] == fields["result_closed"] == "0"
        assert float(fields["sampled_error_fraction"]) < 1e-10
        vertices, uvs, materials = [], [], set()
        material = None
        for line in refined.read_text().splitlines():
            parts = line.split()
            if not parts:
                continue
            if parts[0] == "v":
                vertices.append(tuple(map(float, parts[1:])))
            elif parts[0] == "vt":
                uvs.append(tuple(map(float, parts[1:])))
            elif parts[0] == "usemtl":
                material = parts[1]
            elif parts[0] == "f":
                assert len(parts) == 4
                materials.add(material)
                for corner in parts[1:]:
                    vi, ti = map(int, corner.split("/"))
                    x, y, z = vertices[vi-1]
                    u, v = uvs[ti-1]
                    assert all(math.isfinite(n) for n in (x, y, z, u, v))
                    offset = 10 if material == "right" else 0
                    assert abs(u - x - offset) < 1e-9 and abs(v-y) < 1e-9
                    assert x >= .5 - 1e-9 if offset else x <= .5 + 1e-9
        assert materials == {"surface", "right"}

        # Existing outputs must remain untouched even on failure.
        original = refined.read_bytes()
        run = subprocess.run([str(executable), str(source), str(refined), "0.1"], capture_output=True)
        assert run.returncode == 1 and refined.read_bytes() == original
        for args in (("nan",), ("inf",), (".1", "1.5"), (".1", "0"), (".1", "2", "181")):
            run = subprocess.run([str(executable), str(source), str(root / "invalid.obj"), *args], capture_output=True)
            assert run.returncode == 1, args
            assert not (root / "invalid.obj").exists()

        invalid = subprocess.run([str(executable), str(source), str(root / "bad.obj"), "0"],
                                 text=True, capture_output=True, check=False)
        assert invalid.returncode == 1 and "edge-length fraction" in invalid.stderr

        for name, content in (
                ("empty", "v 0 0 0\n"),
                ("degenerate", "v 0 0 0\nv 1 0 0\nv 2 0 0\nvt 0 0\nf 1/1 2/1 3/1\n")):
            invalid_source = root / (name + ".obj")
            invalid_source.write_text(content)
            run = subprocess.run([str(executable), str(invalid_source), str(root / (name + "-out.obj")), ".1"],
                                 text=True, capture_output=True)
            assert run.returncode == 1 and "CGAL_FAIL" in run.stderr, run.stderr

        make_grid(source)
        lines = []
        for line in source.read_text().splitlines():
            if line.startswith("v "):
                line = "v " + " ".join(str(float(n) * 1e-7) for n in line.split()[1:])
            lines.append(line)
        source.write_text("\n".join(lines) + "\n")
        run = subprocess.run([str(executable), str(source), str(root / "small.obj"), ".2"],
                             text=True, capture_output=True)
        assert run.returncode == 0, run.stderr


if __name__ == "__main__":
    main()
