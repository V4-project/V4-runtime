"""Host-only build path checks; no SDK, Docker daemon or device required."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
RESOLVER = ROOT / "bsp/esp32c6/runtime/cmake/dependencies.cmake"


class DependencyTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="v4-paths-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.project = self.root / "V4-runtime/bsp/esp32c6/runtime"
        (self.project / "cmake").mkdir(parents=True)
        shutil.copy2(RESOLVER, self.project / "cmake/dependencies.cmake")
        self.docker = self.root / "docker"

    def dependency(self, path):
        path.mkdir(parents=True, exist_ok=True)
        (path / "marker.h").touch()
        return path

    def resolve(self, override=None, environment=None, expected=None):
        script = self.root / "check.cmake"
        script.write_text(
            'include("' + str(self.project / "cmake/dependencies.cmake") + '")\n'
            'v4_resolve_dependency(DEP V4_TEST_PATH V4-test "' + str(self.docker) + '" marker.h)\n'
            'if(NOT DEP STREQUAL "' + str(expected) + '")\n'
            'message(FATAL_ERROR "Unexpected dependency: ${DEP}")\nendif()\n')
        env = dict(os.environ)
        env.pop("V4_TEST_PATH", None)
        if environment is not None:
            env["V4_TEST_PATH"] = str(environment)
        args = ["cmake"]
        if override is not None:
            args.append("-DDEP=" + str(override))
        return subprocess.run(args + ["-P", str(script)], cwd="/tmp", env=env,
                              capture_output=True, text=True)

    def test_sibling(self):
        sibling = self.dependency(self.root / "V4-test")
        result = self.resolve(expected=sibling)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_ci_precedes_docker_and_sibling(self):
        self.dependency(self.root / "V4-test")
        self.dependency(self.docker)
        ci = self.dependency(self.project / "_deps/V4-test")
        result = self.resolve(expected=ci)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_docker(self):
        result = self.resolve(expected=self.dependency(self.docker))
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_explicit_relative_path_with_spaces(self):
        explicit = self.dependency(self.project / "custom deps")
        self.dependency(self.docker)
        result = self.resolve(override="custom deps", environment="invalid", expected=explicit)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_environment(self):
        explicit = self.dependency(self.root / "custom")
        result = self.resolve(environment=explicit, expected=explicit)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_invalid_override_does_not_fall_back(self):
        self.dependency(self.docker)
        result = self.resolve(override="missing")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("V4-test not found", result.stderr)

    def test_missing(self):
        result = self.resolve()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("V4-test not found", result.stderr)

    def test_make_from_outside_repository(self):
        result = subprocess.run(["make", "-n", "-f", str(ROOT / "Makefile"),
                                 "esp32c6", "DOCKER=1"], cwd="/tmp",
                                capture_output=True, text=True, check=True)
        self.assertIn('docker compose -f "' + str(ROOT / "bsp/esp32c6/docker-compose.yml")
                      + '" run --rm esp-idf idf.py build', result.stdout)
        native = subprocess.run(["make", "-n", "-f", str(ROOT / "Makefile"), "esp32c6"],
                                cwd="/tmp", capture_output=True, text=True, check=True)
        self.assertIn('cd "' + str(ROOT / "bsp/esp32c6/runtime") + '"', native.stdout)


if __name__ == "__main__":
    unittest.main()
