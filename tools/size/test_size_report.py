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
                    "partition_sha256", "harness_sha256", "compile_options", "link_options",
                    "panic_diagnostics"):
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

    def test_panic_preprocessor_command(self):
        self.assertEqual(size.panic_preprocessor_command('cc -Os -o out.obj -c "/src/panic.cpp"'),
                         ["cc", "-Os", "/src/panic.cpp", "-dM", "-E"])

    def test_linker_option_arguments(self):
        for option in ("-u", "--undefined", "-e", "--entry", "-Xlinker", "-Wl,-u",
                       "-Wl,--undefined", "--wrap", "--defsym", "--version-script", "-z", "-l"):
            with self.subTest(option=option):
                self.assertNotEqual(size.option_profiles(["cc " + option + " first"]),
                                    size.option_profiles(["cc " + option + " second"]))
                with self.assertRaisesRegex(ValueError, "missing argument"):
                    size.option_profiles(["cc " + option])
        self.assertNotEqual(size.option_profiles(["cc -Xlinker -u -Xlinker first"]),
                            size.option_profiles(["cc -Xlinker -u -Xlinker second"]))

    def test_linker_response_arguments(self):
        with tempfile.TemporaryDirectory() as temp:
            response = Path(temp) / "linkflags"
            response.write_text("-u first")
            before = size.option_profiles(["cc @" + str(response)])
            response.write_text("-u second")
            self.assertNotEqual(before, size.option_profiles(["cc @" + str(response)]))

    def test_changed_forced_symbol_rejects_report_comparison(self):
        before = self.report()
        after = self.report()
        before["configuration"]["link_options"] = size.option_profiles(["cc -u first"])
        after["configuration"]["link_options"] = size.option_profiles(["cc -u second"])
        with self.assertRaisesRegex(ValueError, "link_options"):
            self.compare(before, after)
        self.assertNotEqual(size.option_profiles(["cc -Wl,-u,first"]),
                            size.option_profiles(["cc -Wl,-u,second"]))

    def test_effective_experiment_config(self):
        size.verify_sdkconfig("CONFIG_A=y\n# CONFIG_B is not set\n", {"CONFIG_A": "y", "CONFIG_B": "n"})
        for content in ("CONFIG_A=y", "", "# CONFIG_B is not set"):
            with self.subTest(content=content), self.assertRaisesRegex(ValueError, "not effective"):
                size.verify_sdkconfig(content, {"CONFIG_A": "n"})

    def test_experiments_are_mutually_exclusive(self):
        self.assertEqual(size.EXPERIMENTS["static-logs"],
                         {"CONFIG_LOG_TAG_LEVEL_IMPL_NONE": "y", "CONFIG_LOG_DYNAMIC_LEVEL_CONTROL": "n"})
        self.assertEqual(size.EXPERIMENTS["no-coex"], {"CONFIG_ESP_COEX_SW_COEXIST_ENABLE": "n"})
        before = self.report()
        after = self.report()
        before["configuration"]["experiment"] = "default"
        after["configuration"]["experiment"] = "quiet-transport"
        with self.assertRaisesRegex(ValueError, "incompatible"):
            self.compare(before, after)

    def test_panic_compiler_verification(self):
        size.verify_panic_macros("#define V4_PANIC_DIAGNOSTICS 1\n", "on")
        size.verify_panic_macros("#define V4_PANIC_DIAGNOSTICS 0\n", "off")
        for macros, requested in (("", "off"), ("#define V4_PANIC_DIAGNOSTICS 1", "off"),
                                  ("#define V4_PANIC_DIAGNOSTICS 0", "on")):
            with self.subTest(macros=macros), self.assertRaisesRegex(ValueError, "compiler"):
                size.verify_panic_macros(macros, requested)

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

    def test_quiet_logs_combines_only_logging_options(self):
        self.assertEqual(size.EXPERIMENTS["quiet-logs"], size.EXPERIMENTS["static-logs"])
        self.assertNotIn("CONFIG_ESP_COEX_SW_COEXIST_ENABLE", size.EXPERIMENTS["quiet-logs"])
        self.assertEqual(set(size.QUIET_TRANSPORT_EXPERIMENTS), {"quiet-transport", "quiet-logs"})
        before, after = self.report(), self.report()
        before["configuration"]["experiment"] = "static-logs"
        after["configuration"]["experiment"] = "quiet-logs"
        with self.assertRaisesRegex(ValueError, "experiment"):
            self.compare(before, after)

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
