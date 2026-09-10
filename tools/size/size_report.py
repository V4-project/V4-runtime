#!/usr/bin/env python3
"""Isolated ESP32-C6 size builds and strictly comparable reports (Python 3.8+)."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parent.parent.parent
PROJECT = Path("bsp/esp32c6/runtime")
IMAGE = "espressif/idf:v5.5.5"
METRICS = ("application_bin", "bootloader_bin", "diram_data", "diram_bss",
           "used_diram", "flash_code", "flash_rodata", "total_size")


def run(args, cwd=None):
    return subprocess.check_output([str(a) for a in args], cwd=cwd, text=True).strip()


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_json(path, value):
    Path(path).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def snapshot(source, destination):
    """Copy tracked working-tree content only; never include .env/build caches."""
    source = source.resolve()
    if Path(run(["git", "rev-parse", "--show-toplevel"], source)).resolve() != source:
        raise ValueError("source must be a repository root: " + str(source))
    files = run(["git", "ls-files", "-z"], source).split("\0")
    digest = hashlib.sha256()
    for name in sorted(filter(None, files)):
        relative = Path(name)
        if relative.is_absolute() or ".." in relative.parts or relative.name == ".env":
            raise ValueError("unsafe tracked path: " + name)
        item = source / relative
        if item.is_symlink():
            raise ValueError("symlinks are not supported: " + name)
        if not item.exists():  # A tracked deletion is part of the working tree.
            continue
        data = item.read_bytes()
        digest.update(name.encode() + b"\0" + hashlib.sha256(data).digest())
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(item, target)
    return {"commit": run(["git", "rev-parse", "HEAD"], source),
            "dirty": bool(run(["git", "status", "--porcelain", "--untracked-files=no"], source)),
            "tracked_content_sha256": digest.hexdigest()}


def expand_response_files(tokens, responses, depth=0):
    if depth > 10:
        raise ValueError("recursive compiler response file")
    result = []
    for token in tokens:
        if token.startswith("@"):
            path = Path(token[1:])
            content = path.read_text()
            responses[str(path)] = content
            result.extend(expand_response_files(shlex.split(content), responses, depth + 1))
        else:
            result.append(token)
    return result


def option_profiles(commands, responses=None):
    """Compare effective options, without source/object lists or project version."""
    profiles = set()
    if responses is None:
        responses = {}
    for command in commands:
        tokens = iter(expand_response_files(shlex.split(command), responses))
        options = []
        for token in tokens:
            if token in ("-o", "-c", "-MF", "-MT", "-MQ"):
                next(tokens, None)
            elif token.startswith(("-DPROJECT_VER=", "-DV4_RUNTIME_VERSION=")):
                continue
            elif token in ("-D", "-U", "-I", "-isystem", "-include", "-T", "-L",
                           "-isysroot", "--sysroot"):
                argument = next(tokens)
                if token == "-D" and argument.startswith(("PROJECT_VER=", "V4_RUNTIME_VERSION=")):
                    continue
                options.extend((token, argument))
            elif token.startswith("-"):
                options.append(token)
        profiles.add(tuple(options))
    return sorted(profiles)


def collect():
    """Internal command, executed only in the controlled build container."""
    project = Path("/project") / PROJECT
    build = project / "build"
    os.chdir(project)
    subprocess.run(["idf.py", "-B", "build", "reconfigure"], check=True)
    subprocess.run(["ninja", "-C", "build", "-j", os.environ["V4_SIZE_JOBS"]], check=True)
    subprocess.run(["idf.py", "-B", "build", "size"], check=True)
    output = Path("/results")
    for flags, name in (([], "idf-size.json"), (["--archives"], "idf-archives.json")):
        subprocess.run([sys.executable, "-m", "esp_idf_size", "--format", "json",
                        *flags, "-o", str(output / name), str(build / "v4-runtime.map")], check=True)
    commands = json.loads((build / "compile_commands.json").read_text())
    ninja_commands = run(["ninja", "-C", "build", "-t", "commands"])
    (output / "build-commands.txt").write_text(ninja_commands + "\n")
    link_commands = [c for c in ninja_commands.splitlines() if " -o v4-runtime.elf " in c]
    if len(link_commands) != 1:
        raise ValueError("could not identify the application link command")
    for source, name in ((build / "v4-runtime.bin", "application.bin"),
                         (build / "v4-runtime.elf", "application.elf"),
                         (build / "v4-runtime.map", "application.map"),
                         (build / "bootloader/bootloader.bin", "bootloader.bin"),
                         (build / "partition_table/partition-table.bin", "partition-table.bin"),
                         (build / "project_description.json", "project_description.json"),
                         (build / "compile_commands.json", "compile_commands.json"),
                         (project / "sdkconfig", "sdkconfig")):
        shutil.copy2(source, output / name)
    sizes = json.loads((output / "idf-size.json").read_text())
    metadata = json.loads((output / "inputs.json").read_text())
    responses = {}
    config = {"image_id": metadata["image_id"], "harness_sha256": sha(__file__),
              "target": "esp32c6", "profile": "tracked-runtime-defaults",
              "sdk": run(["idf.py", "--version"]),
              "compiler": run(["riscv32-esp-elf-gcc", "--version"]),
              "size_tool": run([sys.executable, "-c",
                                "from importlib.metadata import version; print(version('esp-idf-size'))"]),
              "sdkconfig_sha256": sha(output / "sdkconfig"),
              "partition_sha256": sha(output / "partition-table.bin"),
              "compile_options": option_profiles((c["command"] for c in commands), responses),
              "link_options": option_profiles(link_commands, responses),
              "dependencies": {k: v["tracked_content_sha256"]
                               for k, v in metadata["sources"].items() if k != "runtime"}}
    metrics = {k: sizes[k] for k in METRICS if k not in ("application_bin", "bootloader_bin")}
    metrics.update(application_bin=(output / "application.bin").stat().st_size,
                   bootloader_bin=(output / "bootloader.bin").stat().st_size)
    report = {"schema": 1, "configuration": config, "sources": metadata["sources"],
              "metrics": metrics, "artifacts_sha256": {
                  name: sha(output / name) for name in ("application.bin", "application.elf",
                                                       "application.map", "bootloader.bin")}}
    write_json(output / "response-files.json", responses)
    write_json(output / "report.json", report)
    print(json.dumps(metrics, indent=2))


def build(args):
    sources = {"runtime": args.source.resolve(), "engine": args.engine.resolve(),
               "hal": args.hal.resolve(), "link": args.link.resolve()}
    output = args.output.resolve()
    for source in sources.values():
        if output == source or source in output.parents:
            raise ValueError("output must be outside source repositories")
    if output.exists() and any(output.iterdir()):
        raise ValueError("output must be new or empty; existing builds are never deleted")
    image_id = run(["docker", "image", "inspect", args.image, "--format", "{{.Id}}"])
    output.mkdir(parents=True, exist_ok=True)
    metadata = {"image_id": image_id, "image_requested": args.image, "sources": {}}
    for name, source in sources.items():
        metadata["sources"][name] = snapshot(source, output / "sources" / name)
    # Configs generated by the SDK must never be copied into a new measurement.
    project = output / "sources/runtime" / PROJECT
    for name in ("sdkconfig", "sdkconfig.old", "dependencies.lock", "build", "managed_components", "_deps"):
        if (project / name).exists():
            raise ValueError("generated input unexpectedly tracked: " + name)
    results = output / "results"
    results.mkdir()
    write_json(results / "inputs.json", metadata)
    # Freeze this invocation's harness, including uncommitted harness changes.
    shutil.copy2(__file__, output / "harness.py")
    command = ["docker", "run", "--rm", "--network", "none", "--cpus", str(args.jobs),
               "--user", str(os.getuid()) + ":" + str(os.getgid()),
               "-e", "V4_SIZE_JOBS=" + str(args.jobs)]
    mounts = [(output / "sources/runtime", "/project", False),
              (results, "/results", False), (output / "harness.py", "/harness.py", True)]
    mounts += [(output / "sources" / name, "/v4-" + name, True)
               for name in ("engine", "hal", "link")]
    for source, target, readonly in mounts:
        if "," in str(source):
            raise ValueError("Docker mount paths cannot contain commas")
        command += ["--mount", "type=bind,src=" + str(source) + ",dst=" + target
                    + (",readonly" if readonly else "")]
    command += [image_id, "python", "/harness.py", "_collect"]
    print("Building; log: " + str(output / "build.log"), flush=True)
    with (output / "build.log").open("w") as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
    print("Report: " + str(results / "report.json"))


def compare(before, after, max_growth=None):
    for report in (before, after):
        if report.get("schema") != 1 or not report.get("configuration"):
            raise ValueError("unsupported or missing report schema/configuration")
        if set(report["metrics"]) != set(METRICS):
            raise ValueError("unexpected metric set")
        if any(type(v) is not int or v < 0 for v in report["metrics"].values()):
            raise ValueError("metrics must be nonnegative integers")
    if before["configuration"] != after["configuration"]:
        keys = sorted(k for k in set(before["configuration"]) | set(after["configuration"])
                      if before["configuration"].get(k) != after["configuration"].get(k))
        raise ValueError("incompatible configuration: " + ", ".join(keys))
    print("| Metric (bytes) | Before | After | Delta |\n|---|---:|---:|---:|")
    for key in METRICS:
        a, b = before["metrics"][key], after["metrics"][key]
        print("| {} | {} | {} | {:+d} |".format(key, a, b, b - a))
    growth = after["metrics"]["application_bin"] - before["metrics"]["application_bin"]
    return 1 if max_growth is not None and growth > max_growth else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    p = sub.add_parser("build")
    p.add_argument("--source", type=Path, default=ROOT)
    for name in ("engine", "hal", "link"):
        p.add_argument("--" + name, type=Path, default=ROOT.parent / ("V4-" + name))
    p.add_argument("--image", default=IMAGE)
    p.add_argument("--jobs", type=int, default=4)
    p.add_argument("--output", type=Path, required=True)
    p = sub.add_parser("compare")
    p.add_argument("before", type=Path)
    p.add_argument("after", type=Path)
    p.add_argument("--max-growth", type=int)
    sub.add_parser("_collect", help=argparse.SUPPRESS)
    args = parser.parse_args()
    try:
        if args.action == "build":
            if args.jobs < 1:
                raise ValueError("jobs must be positive")
            build(args)
        elif args.action == "_collect":
            collect()
        else:
            return compare(json.loads(args.before.read_text()), json.loads(args.after.read_text()),
                           args.max_growth)
        return 0
    except (OSError, ValueError, KeyError, TypeError, subprocess.CalledProcessError) as error:
        print("error: " + str(error), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
