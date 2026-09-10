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
validate board behavior. Task and panic-output settings are not changed by the tool.
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
