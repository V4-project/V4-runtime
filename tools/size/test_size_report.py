import contextlib
import io
from pathlib import Path
import re
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import size_report as size


class ReporterTests(unittest.TestCase):
    def test_ci_dependency_pins_match_firmware_build(self):
        workflow = size.ROOT / ".github/workflows"
        pins = lambda name: re.findall(r"ref: ([0-9a-f]{40})", (workflow / name).read_text())
        self.assertEqual(len(pins("ci.yml")), 3)
        self.assertEqual(pins("ci.yml"), pins("size.yml"))
        self.assertIn(size.IMAGE, (workflow / "size.yml").read_text())

    def report(self):
        return {"schema": 1, "configuration": {"sdk": "5.5.5"},
                "metrics": dict.fromkeys(size.METRICS, 100)}

    def compare(self, *args):
        with contextlib.redirect_stdout(io.StringIO()):
            return size.compare(*args)

    def test_same_report(self):
        self.assertEqual(self.compare(self.report(), self.report(), 0), 0)

    def test_growth_report_only_and_budget(self):
        after = self.report()
        after["metrics"]["application_bin"] += 4
        self.assertEqual(self.compare(self.report(), after), 0)
        self.assertEqual(self.compare(self.report(), after, 3), 1)
        self.assertEqual(self.compare(self.report(), after, 4), 0)

    def test_ram_does_not_use_flash_budget(self):
        after = self.report()
        after["metrics"]["diram_bss"] += 100
        self.assertEqual(self.compare(self.report(), after, 0), 0)

    def test_configuration_mismatch(self):
        for key in ("sdk", "dependencies", "image_id", "sdkconfig_sha256",
                    "partition_sha256", "harness_sha256", "compile_options", "link_options"):
            after = self.report()
            after["configuration"][key] = "changed"
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, "incompatible"):
                self.compare(self.report(), after)

    def test_invalid_reports(self):
        for key, value in (("schema", 2), ("configuration", {}), ("metrics", {})):
            after = self.report()
            after[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.compare(self.report(), after)
        for value in (-1, True, "100", 1.5):
            after = self.report()
            after["metrics"]["application_bin"] = value
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.compare(self.report(), after)

    def test_options_ignore_source_output_and_project_version(self):
        a = 'cc -Os -DPROJECT_VER=\\"0.4.0\\" -c a.cpp -o a.obj'
        b = 'cc -Os -DPROJECT_VER=\\"0.5.0\\" -c b.cpp -o b.obj'
        self.assertEqual(size.option_profiles([a]), size.option_profiles([b]))
        self.assertNotEqual(size.option_profiles([a]), size.option_profiles([b + " -flto"]))

    def test_runtime_version_and_separate_option_arguments(self):
        self.assertEqual(size.option_profiles(['cc -DV4_RUNTIME_VERSION="0.4.0" -D PROJECT_VER=1 -Os']),
                         size.option_profiles(['cc -DV4_RUNTIME_VERSION="0.5.0" -D PROJECT_VER=2 -Os']))
        self.assertNotEqual(size.option_profiles(['cc -D FEATURE=0 -T first.ld']),
                            size.option_profiles(['cc -D FEATURE=1 -T second.ld']))

    def test_response_file_flags_are_compared(self):
        with tempfile.TemporaryDirectory() as temp:
            response = Path(temp) / "flags"
            response.write_text("-march=rv32imac -Os")
            a = size.option_profiles(['cc @"' + str(response) + '" -c file.c'])
            response.write_text("-march=rv32imac -O2")
            b = size.option_profiles(['cc @"' + str(response) + '" -c file.c'])
            self.assertNotEqual(a, b)
            response.write_text('@"' + str(response) + '"')
            with self.assertRaisesRegex(ValueError, "recursive"):
                size.option_profiles(['cc @"' + str(response) + '"'])

    def test_snapshot_tracks_edits_but_not_untracked_files(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "source"
            source.mkdir()
            subprocess.run(["git", "init", "-q", str(source)], check=True)
            (source / "code").write_text("original")
            subprocess.run(["git", "-C", str(source), "add", "code"], check=True)
            subprocess.run(["git", "-C", str(source), "-c", "user.name=Test", "-c",
                            "user.email=test@example.invalid", "commit", "-qm", "initial"], check=True)
            (source / ".env").write_text("must not be copied")
            before = size.snapshot(source, root / "before")
            (source / "code").write_text("modified")
            after = size.snapshot(source, root / "after")
            self.assertFalse(before["dirty"])
            self.assertTrue(after["dirty"])
            self.assertNotEqual(before["tracked_content_sha256"], after["tracked_content_sha256"])
            self.assertFalse((root / "after/.env").exists())
            self.assertEqual((root / "after/code").read_text(), "modified")
            (source / "code").unlink()
            size.snapshot(source, root / "deleted")
            self.assertFalse((root / "deleted/code").exists())

    def test_existing_output_rejected_before_docker(self):
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp)
            (output / "keep").write_text("keep")
            args = type("Args", (), dict(source=Path("/source"), engine=Path("/engine"),
                                        hal=Path("/hal"), link=Path("/link"), output=output))()
            with patch.object(size, "run") as run, self.assertRaisesRegex(ValueError, "empty"):
                size.build(args)
            run.assert_not_called()
            self.assertEqual((output / "keep").read_text(), "keep")

    def test_output_inside_source_rejected_before_docker(self):
        args = type("Args", (), dict(source=Path("/source"), engine=Path("/engine"),
                                    hal=Path("/hal"), link=Path("/link"),
                                    output=Path("/source/build-size")))()
        with patch.object(size, "run") as run, self.assertRaisesRegex(ValueError, "outside"):
            size.build(args)
        run.assert_not_called()


if __name__ == "__main__":
    unittest.main()
