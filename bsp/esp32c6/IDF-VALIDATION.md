# ESP-IDF 5.5.5 validation

Date: 2026-09-10. Target: ESP32-C6 / NanoC6 runtime. **No hardware was used.**

## Scope and reproducibility

Compared the same runtime source (`432cfa9c98710313bf9028daed1859bfc403a8ce`)
under ESP-IDF 5.3.0 and 5.5.5 in separate, initially empty build directories.
Both used the tracked `runtime/sdkconfig.defaults`, not a developer's generated
sdkconfig or the optional board configuration. SDK defaults and toolchains change
with IDF; this measures the SDK migration, not an engine-only optimization.

Identical dependency source was used for both:

| Dependency | Commit |
|---|---|
| V4-engine | `a7eb42611170091c72b7a17799b57c13f62fa6d1` (0.18.1 source) |
| V4-hal | `d36a55ac4782eed526c03afde9236c53f5bc84fa` |
| V4-link | `d155eefabda98ca6716cbe0f81dd8e602d41c990` |

The first attempts used engine 0.18.0 and failed in both SDKs: `int32_t` is `long`
on these RISC-V toolchains, but panic output used `%d`. Both comparison builds
were then completed with the identical `PRId32` fix released as engine 0.18.1.
This was not an IDF 5.5-only regression. Engine host tests also pass after the fix.

Official Docker image digests:

- `espressif/idf:v5.3`: `sha256:89df2532b2f5df278a8cca6a06358f6e615628aabeb0f7e639e27646a23208d7`
- `espressif/idf:v5.5.5`: `sha256:a9231d0697ab8f7517cc072e93b7c83e04907bfbfba80b6440d7dbbf90665cf2`

The SDKs reported GCC 13.2.0 (`esp-13.2.0_20240530`) and GCC 14.2.0
(`esp-14.2.0_20260121`), respectively. Containers had no network or device access;
dependency mounts were read-only. From `bsp/esp32c6/runtime`, the build sequence was:

```sh
idf.py --version
riscv32-esp-elf-gcc --version
idf.py -B build reconfigure
ninja -C build -j 4
idf.py -B build size
idf.py -B build size-components
riscv32-esp-elf-size --format=berkeley build/v4-runtime.elf
```

## Comparison results

Both SDK builds and their bootloader/application partition-size checks passed.

A final independent clean build also passed with runtime version 0.4.0, the
strict `==5.5.5` component requirement, and the committed engine 0.18.1 checkout.
Its application `.bin` remained 139,408 bytes and generated sdkconfig SHA-256 was
`1efc3c3a6f4275ab9409e34419f0c447a28d6bdb77c626b61d56ad5f3dbfd2c3`.
The application `.bin` SHA-256 was
`db767576c95c2d8ecd4ff275eeabb74b24ab270a91b71d8f466b58f8320463f8`.
Build timestamps and metadata may change binary hashes in subsequent builds.

| Metric (bytes) | IDF 5.3.0 | IDF 5.5.5 | Delta |
|---|---:|---:|---:|
| Application `.bin` | 135,328 | 139,408 | +4,080 |
| Bootloader `.bin` | 20,096 | 20,576 | +480 |
| IDF-reported DIRAM use | 61,036 | 65,394 | +4,358 |
| DIRAM `.bss` | 20,344 | 20,352 | +8 |
| DIRAM `.data` | 3,769 | 3,956 | +187 |

The application partition is 1,048,576 bytes, leaving 909,168 bytes after the
5.5.5 application image. The migration increases size; it is adopted as an SDK
maintenance update, not advertised as a size optimization. IDF size-tool versions
and memory classifications differ; DIRAM includes code and static data and is not
a measurement of runtime free heap. Do not use the full ELF file length as flash use.

## Warnings and configuration caveats

- Existing warning in both SDKs: engine's FreeRTOS wrapper has an unused
  `word_idx`; the word-execution call in that wrapper is still a placeholder.
  This migration does not complete or validate that scheduling behavior.
- Additional non-fatal warning in 5.5.5: HAL's aggregate `uart_config_t`
  initialization omits the new `flags` member. Omitted aggregate members are
  zero-initialized; no explicit UART flag behavior is enabled by this migration.
  A HAL portability cleanup remains separate work.
- Both generated configurations enable `CONFIG_ESP_WIFI_ENABLED`, despite the
  attempted `=n` in the old defaults. Do not claim the Wi-Fi component is excluded
  merely from that defaults line. Its effective configuration is unchanged here.
- `CONFIG_FREERTOS_HZ=1000`, main stack 8192 bytes, 4 MB flash, USB Serial/JTAG
  console, size optimization and silent assertions remain selected. IDF 5.5.5
  uses Log V1. No Log V2, task removal or panic-output removal was enabled.
- The old comment describing silent assertions as LTO was incorrect. It is
  corrected without changing the compiler/SDK optimization settings.

## Hardware validation still required

Compilation does not validate boot, USB protocol traffic, actual task scheduling,
panic recovery or LED indication. Flashing, board identity/revision, electrical
behavior, runtime heap/stack use and long-running stability were not tested.
Run those checks on the intended NanoC6 before claiming hardware readiness.

The CI workflow pins SDK/dependency versions and preserves firmware, ELF, map,
generated sdkconfig and project metadata. Developer-generated configs and lockfiles
are intentionally not overwritten; use a fresh worktree/build when reproducing
this comparison rather than reusing a 5.3 CMake cache or sdkconfig.
