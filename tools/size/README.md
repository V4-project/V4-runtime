# ESP32-C6 firmware size comparisons

Python 3.8+, Git and Docker are required. No hardware, serial device, SSH keys or
network access inside the build container is needed. The host must already have
the SDK image (`docker pull espressif/idf:v5.5.5`). Dependency repositories default
to the sibling V4-engine, V4-hal and V4-link checkouts.

From the runtime repository:

```sh
python3 tools/size/size_report.py build --output /tmp/v4-size-before
# Make a runtime change, then use a NEW output directory:
python3 tools/size/size_report.py build --output /tmp/v4-size-after
python3 tools/size/size_report.py compare \
  /tmp/v4-size-before/results/report.json /tmp/v4-size-after/results/report.json
```

`make size-build SIZE_OUTPUT=/tmp/v4-size-before` is equivalent. The existing
`make size` remains a quick display of an existing developer build.

Use `--source /path/to/runtime-baseline` to build another revision using the
**current harness**. `--engine`, `--hal`, `--link`, `--image` and `--jobs` are explicit
overrides. The image is resolved to its immutable local image ID before execution.
Both comparison builds must use the same image and dependency content.

## Isolation and scope

The tool copies tracked working-tree files (including staged/unstaged edits and
deletions) into a new output directory **outside all source repositories**. Untracked
files are not copied: use `git add` for a new source file that should be measured.
The harness itself is always taken from the invoking tool, even before it is staged.
Tracked symlinks, `.env`, and generated runtime configuration/build inputs are rejected.
Developer sdkconfig, dependencies.lock, ignored build caches and `.env` are not used
or overwritten. Existing nonempty outputs are rejected, never cleaned automatically.

This profile uses tracked `runtime/sdkconfig.defaults`, not the optional board
sdkconfig. It builds the current NanoC6 runtime for ESP32-C6; it does not flash or
validate board behavior. Task settings are not changed. Standard engine panic
diagnostics default to ON; an explicit OFF measurement is described below.
Dependencies are read-only in the container; build/results are isolated writable
copies. Failed builds keep their log and partial output for diagnosis.

## Reports and comparison rules

`results/` retains application BIN/ELF/map, bootloader BIN, partition table,
project metadata, generated sdkconfig, compile commands, Ninja commands, compiler
response-file contents, IDF JSON
summary and per-archive sizes, input source identities, and `report.json`.
`build.log` contains the full build and the normal `idf.py size` report.

Comparison requires identical harness, Docker image, SDK/compiler/size-tool,
dependency content, effective sdkconfig, partition table and compiler/linker option
profiles. Runtime source identity is recorded but allowed to differ. Source/object
lists and PROJECT_VER / V4_RUNTIME_VERSION are excluded from option profiles so ordinary source and
version changes can be compared. Response files are expanded before comparing
options, including the SDK's toolchain flags. Profiles are sets of option sequences, not a
per-function attribution or a proof of semantic equivalence. Raw commands remain
available for review. Build timestamps/hashes may differ even at equal size.

Different configurations are rejected; there is intentionally no force override.
For feature on/off or SDK changes, retain separate reports and label the comparison
as a configuration tradeoff instead of a same-configuration refactoring delta.

- `application_bin` is the actual application image length, the optional budget metric.
- `bootloader_bin` is separate; the application figure is not total device flash use.
- DIRAM data/BSS/use and flash code/rodata come from ESP-IDF's map-size parser.
- `total_size` is the IDF parser's image estimate, not the application BIN length.
- DIRAM includes code and static data; none of these metrics measure runtime free
  heap or task stack high-water marks. Full ELF file length is not a flash budget.

By default growth is report-only. `compare ... --max-growth 0` fails on any positive
application BIN growth. Exit status: 0 success, 1 budget exceeded, 2 invalid input,
configuration mismatch or failed build. RAM and bootloader deltas remain visible
but do not use the application budget.

## CI and tests

The Firmware Size workflow builds base/current with the current harness and the
same pinned dependencies. Pull requests use their base SHA, pushes use the previous
SHA, and manual runs take a baseline ref. It publishes a Markdown job summary and
retains reports, logs and firmware artifacts for 30 days. Growth is report-only;
incompatible configurations fail visibly rather than producing a misleading delta.
CI also retains a current OFF build as a separate configuration artifact.

## Standard panic-output opt-out

```sh
python3 tools/size/size_report.py build --panic-diagnostics on --output /tmp/v4-panic-on
python3 tools/size/size_report.py build --panic-diagnostics off --output /tmp/v4-panic-off
```

Both use the same runtime/dependencies/SDK. The tool preprocesses engine `panic.cpp`
with its actual compile command and verifies the requested macro is 1 or 0;
`panic-macros.txt` is retained. An older runtime that ignores OFF is rejected.
The effective feature is recorded in configuration identity. `compare` intentionally
rejects ON vs OFF: this is a diagnostic-feature tradeoff, not a same-configuration
code regression. Inspect the two metric sets and archive/map reports separately.
For subsequent code changes, compare OFF vs OFF (or ON vs ON) with the same harness.

For a native IDF build, from `bsp/esp32c6/runtime`:

```sh
idf.py -DV4_PANIC_DIAGNOSTICS=OFF reconfigure
idf.py build
# Restore default output explicitly in this cached build:
idf.py -DV4_PANIC_DIAGNOSTICS=ON reconfigure
```

This removes only engine's standard printf formatter, including its return-stack
call-trace printing. Snapshots and the custom callback remain, so runtime's own
ESP_LOG diagnostics and LED indication are still compiled. It does not disable
ESP-IDF panic handling, assertions, VM boundary checks or V4-link VM_ERROR replies.
No promise of target panic recovery is made without hardware testing.

Measured on 2026-09-10 with identical runtime working-tree snapshots, pinned
engine/HAL/link and IDF 5.5.5 image. Only the panic option and its effective
compiler definitions differ; sdkconfig and partition table match.

| Metric (bytes) | ON | OFF | Delta |
|---|---:|---:|---:|
| Application BIN | 139,408 | 138,960 | -448 |
| Bootloader BIN | 20,576 | 20,576 | 0 |
| DIRAM use | 65,394 | 65,394 | 0 |
| DIRAM data / BSS | 3,956 / 20,352 | 3,956 / 20,352 | 0 / 0 |
| Flash code | 73,800 | 73,344 | -456 |
| Flash rodata | 16,796 | 16,572 | -224 |
| IDF map image estimate | 136,134 | 135,454 | -680 |

Map estimates and BIN lengths differ due to image layout/padding; do not describe
the 680 B map reduction as the BIN saving. OFF's ELF retains `handle_panic`,
`panic_handler_init`, `vm_panic`, `vm_set_panic_handler`, and the runtime's error
log strings; engine's standard panic banner is absent. Engine host CTest passes
14/14 in each mode, and link CTest passes 3/3 with diagnostics OFF, including error
response after callback return. These checks do not execute ESP32 firmware.
The same harness also clean-builds the old 0.4.0 runtime with inherited ON
diagnostics; baseline/current ON reports pass strict comparison with zero growth.

```sh
python3 -m unittest discover -s tools/size -v
```

See [IDF validation](../../bsp/esp32c6/IDF-VALIDATION.md) for the previous SDK migration
and outstanding hardware/configuration caveats.

Local validation on 2026-09-10: the 0.4.0 baseline (`26ccaaa`) and 0.5.0 working tree
both passed fresh builds with the same final harness, IDF 5.5.5 and dependencies
listed in the runtime README. Strict comparison with `--max-growth 0` passed:
application 139,408 B, bootloader 20,576 B, DIRAM use 65,394 B, all reported deltas
zero. Twelve reporter tests passed. No hardware or remote workflow execution was
part of this local validation.
