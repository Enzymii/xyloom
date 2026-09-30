<p align="right"><a href="passport-v0-validation.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Milestone A visual revision and status validation

Historical validation of the Cottage Milestone A visual revision on 2026-09-30. Results below apply to the exact firmware hashes recorded here, not automatically to later builds. Upstream baseline: `0b9e4c81ee4421c0bac39ca3561d65a8285acd4a`.

| Check | Result | Evidence |
| --- | --- | --- |
| Build | PASS | Complete `tools/validate.sh`, ESP-IDF v5.5.3, exit 0 |
| Host tests | PASS | Complete repository suite, input/world tests including status entry/exit and standby |
| LVGL rendering and fonts | PASS | LVGL 9.5.0, 24 KB allocator, nine frames, actual Chinese glyph descriptors plus negative control |
| Image and archive | PASS | Merged image and partition verification; content-addressed archive independently verified |
| Simulator | PASS for observed checks | New firmware boot, welcome dialog, local focus, status, both scene directions, long-release exclusivity, first-key wake consumption |
| Flash and startup | PASS | Flash data hash verified; bounded 15-second reset/startup capture matched the ELF and reached the application |
| Device tests | NOT RUN | Full visual/input/battery acceptance still awaits user observations |

Unverified: new physical screen colors, readability, smoothness, key timing, battery gauge and memory after worker initialization. Simulator timing and power behavior are not hardware measurements. After idle, the first OK preserved the scene without opening Status; the second OK opened it. The emulator did not visibly dim the backlight, so actual backlight shutdown remains unverified.

## Exact firmware identity

- Full image: 1,131,488 bytes, offset `0x0`; target ESP32-C3, 8 MB Flash.
- Application: 1,065,952 bytes in the 8,323,072-byte factory partition.
- Full-image SHA-256: `831f7d1d22a0f48093ee56fe51ecea4356b5559a55790b0640f91d19697ab4ad`.
- ELF SHA-256: `131707f865bf41374e95d9f0e7c2d5656da45f29002ddf46fdbc680d2d720d0d`.
- Embedded version: `0b9e4c8-dirty`; SDK `v5.5.3`.
- Archive: `build/firmware/831f7d1d22a0f48093ee56fe51ecea4356b5559a55790b0640f91d19697ab4ad/`.

The exact revision was flashed on 2026-09-30 from `0x0`. Only covered sectors `0x0` through `0x114fff` were erased. The reset/startup capture found no panic or repeated reboot; it confirmed CW2017 detection and the existing battery profile match. Startup heap before battery-worker initialization was 246,164 bytes, largest block 114,688 bytes. Actual SOC display remains unverified. A merged-image flash replaces firmware and may reset NVS; see the [flashing policy](../development/engineering/firmware-layout.md#flashing-and-stored-data).

## Visual changes and status

The wooden floor extends beneath the character. The reference-derived character keeps a natural standing pose, a larger head, a simpler expression and softer hair. The orange tulip and coral-red can use cartoon silhouettes. Small ground highlights replace bounding boxes; Status uses an underlined label.

The house's second focus opens Status with OK; OK closes it. The page shows battery, boot uptime and the current character state. A separate worker reads the existing BSP gauge every 10 seconds. Unknown readings show --%. Host snapshots use synthetic battery/uptime values; the emulator shows missing battery because it does not model the gauge.

## Emulator workflow

Source: [VOID001/FoloToy-Passport-Simulator](https://github.com/VOID001/FoloToy-Passport-Simulator), downloaded main archive SHA-256 `60f507b6721381bb80abe02c82d8e412380b7fb5f3caa1c904b29786c2a894b1`. Local-only checkout under ignored `work/simulator-source/FoloToy-Passport-Simulator-main`. The board runtime verifier passed. Launch with `node server.mjs --allow-local-firmware-upload`, then open `http://127.0.0.1:4190` and select the exact full image. Refreshing restores the simulator's default demo, so reselect the local firmware.

The simulator runs the real merged firmware in its WASM/QEMU runtime. It is a development tool, not a new product web frontend or backend. Two local test buttons hold/release the emulated UP/DOWN electrical levels through its existing runtime API; firmware and BSP are unchanged for simulation. Normal simulator short clicks and OK are used for other actions. The boot UART ELF prefix `131707f86` matches the verified archive. UART capture stops before application logging, so it is not a full application-log check.

Tests observed correct endpoints and a frame during the doorway movement; this does not establish real-time frame rate. Battery, BLE and exact low-power behavior are simulator limitations. Confirm real readings and physical display on the device later.

## Prior device feedback and remaining scope

The previous image `e7e41ffa53a6965c82154b7dd162a4c64e74ee53d132a104f5bb88807e81dbe7` received positive device feedback for the tested functions. That feedback does not validate this newer image.

Full v0 drink/watering transactions, growth stages, NVS persistence, night art, sleep pose and autonomous outings remain pending. The tulip is currently a fixed visual preview; growth scheduling is not implemented.


## Daily growth and networking validation (2026-10-01)

- Build: NOT RUN — the complete firmware gate is in progress.
- Host tests: PASS — complete suite, daily cap/excess cups, 14-day minimum, calendar boundaries, bloom recording, migration, clock guards, frozen cross-midnight retries and Wi-Fi form decoding.
- LVGL rendering and fonts: PASS — 29 snapshots including all plant stages, daily UI, Wi-Fi setup/status/clearing and waiting for time; actual glyph coverage and missing-glyph negative control.
- Device tests: NOT RUN — this new firmware has not been flashed.
- Unverified: actual Wi-Fi provisioning/reconnection/clearing, SNTP/date rollover, screen/input behavior, NVS migration/reboot retention and available internal heap during networking.

Networking reference inspected: upstream `demo/blufi-provisioning` at `9c039cc5127f22072afa83bedb7fa3d8efe635ad`. Only stack/lifecycle patterns were consulted; this app uses its own WPA2 hotspot and local Web setup, with Bluetooth disabled.

## Manual watering validation (2026-09-30)

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0.
- Host tests: PASS — complete suite, watering timing/input guards, retry/duplicate prevention, record format and actual NVS-worker fault injection.
- LVGL rendering and fonts: PASS — 18 snapshots, including flight/pouring/return, saving, failure, completion, maximum count and unavailable records; actual Chinese glyph checks plus negative control.
- Device tests: NOT RUN — watering interaction acceptance awaits user observations; segmented flashing and startup checks passed on COM3.
- Unverified: physical animation, new text readability, real NVS save/reboot recovery and interrupted-write behavior.

Full image: 1,160,512 bytes, SHA-256 `a9ac0d58f4456be2dd8a606d2ad67f125908d1f838b73a29908b542515602c76`. Matching ELF SHA-256: `ab291f3774db0db4ae6195abe3b0cc7aee7f5e057dbe9a77c1ce258da7907f5b`. Embedded version: `0125d4a-dirty`; SDK `v5.5.3`. Archive: `build/firmware/a9ac0d58f4456be2dd8a606d2ad67f125908d1f838b73a29908b542515602c76/`, independently verified with `tools/archive_firmware.py verify`. Generated firmware and debug files stay outside Git.

With explicit user approval, the verified archive components were flashed on COM3 at `0x0`, `0x8000` and `0x10000`; all three write hashes passed. The NVS region (`0x9000`–`0xEFFF`) was outside the erased/write ranges. A deliberate reset followed by a 15-second startup capture confirmed ESP-IDF 5.5.3, version `0125d4a-dirty`, matching embedded ELF hash prefix `ab291f377`, application/LVGL initialization and no observed panic, watchdog or storage error. The serial port was closed after capture. Raw logs remain local under ignored `work/`.

The existing 8 MB partition layout is unchanged. Segmented flashing preserves the NVS partition. Flashing the merged image at `0x0` may reset saved watering counts. Identify the device and obtain approval before either operation.
