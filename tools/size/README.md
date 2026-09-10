# ESP32-C6 firmware size comparisons

## Logging defaults and diagnostic restoration

New runtime builds now use the measured `quiet-logs` behavior by default. Standard
panic diagnostics, startup INFO, ERROR and wireless coexistence remain enabled.
Per-tag/runtime log level adjustment is disabled, and transfer counts/hex dumps
are DEBUG-only. Other logs still share USB; this is not a log-free protocol channel.

`build --experiment default` follows the checked-out source's defaults, including
old revisions with verbose defaults. `--experiment diagnostic-logs` explicitly
restores cache+linked-list dynamic logging and INFO traffic traces for diagnosis.
`static-logs` explicitly keeps INFO traffic, `quiet-transport` explicitly restores
dynamic logging, and `quiet-logs` explicitly selects both quiet options. `no-coex`
only changes coexistence relative to the checked-out defaults (now quiet logging).
Historical measurements below used the earlier defaults and are not new no-coex
measurements under the adopted defaults.

```sh
python3 tools/size/size_report.py build --output /tmp/v4-adopted-default
python3 tools/size/size_report.py build --experiment diagnostic-logs --output /tmp/v4-diagnostic
```

Existing developer sdkconfig/CMake caches are **not** automatically migrated or
overwritten. In `idf.py menuconfig`, select Log output → Log Level → Level Settings:
tag level checks **None**, dynamic level control **disabled**. Then run
`idf.py -DV4_LINK_VERBOSE_LOGS=OFF reconfigure` and `idf.py build`.
For the old diagnostic behavior, select **Cache + Linked List**, dynamic level
control **enabled**, and reconfigure with `-DV4_LINK_VERBOSE_LOGS=ON` before rebuilding.
Do not delete or overwrite developer configs to adopt the new defaults. Fresh
isolated size builds apply the tracked defaults without any manual migration.

Normal `compare` is still strict. CI uses `--report-config-change`: when valid
reports have different configurations, it names the changed fields and shows
absolute sizes separately, with **no delta or growth-budget verdict**. Matching
configurations still get the usual delta. Invalid reports fail. This reporting
option cannot be combined with `--max-growth`; it does not make differing builds
comparable. Review both configuration artifacts on default/SDK changes.

Adoption validation (2026-09-10): clean `default` produces 137,568 B application
BIN, 3,932 B data, 20,072 B BSS and 64,574 B DIRAM use. `diagnostic-logs` restores
139,408 B / 3,956 B / 20,352 B / 65,394 B respectively; both bootloaders are 20,576 B.
The restored mode and new default use identical source snapshots. The old 0.4.0
runtime also builds under the current harness with its old verbose defaults.
The measured reduction from old logging behavior is 1,840 B application BIN and
304 B static data+BSS, not a same-configuration code-only improvement.
Strict comparison rejects old/new defaults; CI's reporting mode shows separate
absolute sizes and no delta. Twenty-three reporter tests and formatting checks
pass. Hardware and remote CI execution are still pending. ELF inspection confirms
startup/error/standard panic output, custom callback and coexistence remain linked.

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
incompatible configurations are shown separately without a misleading delta when
CI's explicit configuration-change reporting mode is selected.
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

## Independent configuration experiments

Use the same source and default panic setting for each run. Select one named
profile per build; `quiet-logs` explicitly combines the logging changes, while
`default` follows the checked-out defaults (now also quiet):

```sh
python3 tools/size/size_report.py build --experiment default --output /tmp/v4-config-default
python3 tools/size/size_report.py build --experiment static-logs --output /tmp/v4-config-static-logs
python3 tools/size/size_report.py build --experiment quiet-transport --output /tmp/v4-config-quiet-transport
python3 tools/size/size_report.py build --experiment quiet-logs --output /tmp/v4-config-quiet-logs
python3 tools/size/size_report.py build --experiment no-coex --output /tmp/v4-config-no-coex
```

- `static-logs`: generated defaults overlay selects `CONFIG_LOG_TAG_LEVEL_IMPL_NONE=y`
  and `CONFIG_LOG_DYNAMIC_LEVEL_CONTROL=n`. INFO/ERROR output remains, but runtime
  per-tag/dynamic log level adjustment is unavailable.
- `quiet-transport`: passes `V4_LINK_VERBOSE_LOGS=OFF`. Transfer counts and HEX dumps
  move from INFO to DEBUG; startup, errors and panic reporting stay as before.
  This does not guarantee a log-free binary transport: other logs still share USB.
- `no-coex`: generated defaults overlay selects `CONFIG_ESP_COEX_SW_COEXIST_ENABLE=n`.
  This is only a candidate for the current radio-unused runtime, not a general
  recommendation for future Wi-Fi/BLE/802.15.4 applications. It does not force the
  hidden Wi-Fi enable symbol off or promise radio/power correctness on hardware.

- `quiet-logs`: combines the static-logs SDK overlay and `V4_LINK_VERBOSE_LOGS=OFF`.
  It does not disable wireless coexistence or standard panic output. Both the SDK
  settings and transport compiler macro are checked. Per-tag/runtime log adjustment
  is lost, while ERROR and startup INFO messages remain; transfer tracing requires
  a build whose maximum log level permits DEBUG.

The overlays are created only inside the isolated source copy; tracked defaults
and developer settings are untouched. Generated sdkconfig values are checked after
configuration. Quiet transport additionally checks its compiler macro. Profiles
are recorded and normal `compare` rejects different experiments. Retain separate
reports and describe their deltas as feature tradeoffs. Setting changes can affect
multiple SDK components, so inspect generated sdkconfig diffs as well as metrics.

Firmware Size's manual `experiments` input builds all candidate profiles; normal
push/PR CI does not add these experiment builds. These profiles do not change defaults
for normal firmware builds. For native builds, the quiet transport option is
`idf.py -DV4_LINK_VERBOSE_LOGS=OFF reconfigure`; restore it with ON in a cached build.

The harness records symbol arguments to `-u` and related linker options, including
response files. Older reports omit these arguments: rebuild both sides with this
harness rather than comparing or editing old report metadata.

### Measured results (2026-09-10)

Four clean builds used identical tracked source snapshots, IDF 5.5.5 and pinned
dependencies, with standard panic diagnostics ON throughout. Each experiment
changes only the feature described above and its SDK-derived configuration.

| Profile | App BIN (B) | Delta (B) | Data (B) | BSS (B) | DIRAM use (B) |
|---|---:|---:|---:|---:|---:|
| default | 139,408 | 0 | 3,956 | 20,352 | 65,394 |
| static-logs | 138,176 | -1,232 | 3,956 | 20,072 | 64,850 |
| quiet-transport | 138,800 | -608 | 3,940 | 20,352 | 65,134 |
| no-coex | 136,960 | -2,448 | 3,564 | 20,352 | 64,188 |

Bootloader BIN remains 20,576 B in all four. Static data+BSS decrease by 280 B,
16 B and 392 B respectively. DIRAM reductions also include code and must not be
presented as runtime free-heap measurements. Savings are **not generally additive**:
shared code/alignment can change the combined result. This initial table measures
each feature separately.

The static-logs ELF no longer contains `s_log_cache`; no-coex removes `coex_pre_init`
and the coexistence init hook. Quiet transport removes INFO traffic strings at the
default INFO limit, while startup/error strings remain. Runtime panic callbacks
remain linked. The sdkconfig diff for static-logs is confined to tag/dynamic-level
settings; no-coex changes coexistence settings and their compatibility aliases.
All three cross-profile `compare` attempts correctly fail with exit code 2.

Reporter tests pass (19), formatting checks pass, and existing host link tests
pass (3). None of these checks execute firmware on hardware. Candidates remain
opt-in; this initial validation did not test their combination or adopt new defaults.

### Combined logging result (2026-09-10)

Default and `quiet-logs` were subsequently rebuilt from identical tracked sources
using the same updated harness, SDK and dependencies. Standard panic diagnostics
remain ON; wireless coexistence remains enabled. The effective sdkconfig changes
only log tag/dynamic-level settings, and the transport macro is verified as zero.

| Metric (bytes) | Default | quiet-logs | Delta |
|---|---:|---:|---:|
| Application BIN | 139,408 | 137,568 | -1,840 |
| Static data | 3,956 | 3,932 | -24 |
| BSS | 20,352 | 20,072 | -280 |
| DIRAM use | 65,394 | 64,574 | -820 |
| Bootloader BIN | 20,576 | 20,576 | 0 |

Static data+BSS save 304 B; DIRAM also includes code. In this case the BIN saving
equals the sum of earlier individual BIN savings, but static data+BSS save 8 B
more than their individual sum. Measure combinations rather than assuming sums.
The map image estimate drops by 2,538 B, which is not the BIN saving.

ELF inspection confirms removal of the log cache and INFO transfer strings while
retaining `coex_pre_init`, the standard panic banner, runtime panic callback,
startup INFO and error messages. Runtime log level/tag adjustment is unavailable,
and binary transport is still not guaranteed free of other logs. Cross-profile
comparison is rejected as expected. Twenty reporter tests and formatting checks
pass. This was the pre-adoption experiment; it did not change normal firmware
defaults or validate hardware. Current default adoption is documented above.

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
