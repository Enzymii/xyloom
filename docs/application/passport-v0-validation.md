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


## Current-stage percentage display (2026-10-01)

Plant inspection and watering completion show the current stage’s whole completion percentage instead of cumulative x/56. The garden bar uses the same percentage. Seed 0–4, sprout 4–20 and bud 20–56 each start at zero; bloom stays at 100%. Integer percentages round down. Persistence and growth thresholds are unchanged.

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0; archive independently verified.
- Host tests: PASS — complete suite, stage start/midpoint/end boundaries and bloom percentage; 32 rendered snapshots, actual glyph checks and idle-refresh regression.
- Device tests: NOT RUN — this UI update has not been flashed.
- Unverified: physical percentage/bar readability and interactions.

Full image: 1,840,176 bytes, SHA-256 `6b575e6c5705f4033be15325d4de4630e40afa4b716d3159ca9789a2be25fd33`. ELF SHA-256: `3b86971ffd041bfd6a1b9437b07d52dfba92c26f5f7e66fa91da3a5f4aabe11c`. Embedded version: `feb6de2-dirty`; SDK `v5.5.3`. Verified archive: `build/firmware/6b575e6c5705f4033be15325d4de4630e40afa4b716d3159ca9789a2be25fd33/`. No physical device was flashed for this update.

Simulator: PARTIAL — the local VOID001 simulator loaded the unchanged verified image and displayed the house; UART ELF prefix `3b86971ff` matched. UP/OK and explicit held/released button levels produced no observed UI change. A sampled PC decoded to the instruction after WFI in `esp_cpu_wait_for_intr`. An ignored local diagnostic copy replacing only that WFI with NOP, with corrected image checksum/SHA and a bud-stage NVS fixture, also failed to restore interaction. The cause remains unresolved; neither percentage inspection nor transitions are claimed as simulator PASS. The production archive and application sources contain no WFI workaround. The browser was restored to the unchanged image. Percentage screenshots come from the actual LVGL host renderer, not simulated firmware. Raw logs, screenshots and test images stay local under ignored `work/`.

## Setup access and idle rendering repair (2026-10-01)

The previous image rejected requests to the setup AP because the HTTP server returned an IPv4-mapped IPv6 socket address, while the guard accepted only IPv4. The guard now uses a full socket address buffer and recognizes both forms of the AP address; other destinations remain rejected. Host cases cover both allowed forms, LAN destinations, native IPv6 and malformed lengths.

Repeated setters invalidated the stationary view every 20 ms. Watchdog samples from the matching previous ELF were in draw dispatch and label drawing. The repair compares visible state before invoking LVGL setters and changes plant image sources only on stage changes. The rendering regression checks that repeated static garden/Status updates produce no additional flushes, while cup and Status-uptime changes still flush.

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0; archive independently verified.
- Host tests: PASS — repository suite plus 29 rendered snapshots, glyph checks and stationary-refresh regression.
- Device tests: PASS — segmented flashing and 45-second startup observation only; no watchdog, panic, assertion or error log observed.
- Unverified: phone provisioning, normal UI interactions, Wi-Fi/SNTP and day rollover.

With explicit user authorization to repair and flash, the verified archive was written on COM3 at `0x0`, `0x8000` and `0x10000`; all three write hashes passed and the NVS range was excluded. A deliberate reset and 45-second capture confirmed SDK `v5.5.3`, version `feb6de2-dirty` and matching ELF prefix `24fe0a87c`. The previous repeating startup watchdog did not recur within this window. The serial port was closed. Phone setup and physical UI acceptance are deferred to the user; this is not full feature acceptance. Full image: 1,840,112 bytes, SHA-256 `fab28dc27ddd1981d906733ffc058d857c9880bba214bffc08ff01867c30b42e`. ELF SHA-256: `24fe0a87c9905a11fcca116fa1e8352e4cb16fbff1b21b2f19cbdae5ff642094`. Verified archive: `build/firmware/fab28dc27ddd1981d906733ffc058d857c9880bba214bffc08ff01867c30b42e/`. The repair was built from uncommitted sources; its dirty version denotes those changes. Raw logs and generated artifacts remain ignored and local.

## Daily growth and networking validation (2026-10-01)

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0; merged image and archived debug bundle independently verified.
- Host tests: PASS — complete suite, daily cap/excess cups, 14-day minimum, calendar boundaries, bloom recording, migration, clock guards, frozen cross-midnight retries and Wi-Fi form decoding.
- LVGL rendering and fonts: PASS — 29 snapshots including all plant stages, daily UI, Wi-Fi setup/status/clearing and waiting for time; actual glyph coverage and missing-glyph negative control.
- Device tests: FAIL — COM3 segmented flashing passed all three write hashes, but a 15-second startup capture observed repeated task-watchdog warnings for IDLE CPU 0 while taskLVGL was running.
- Unverified: actual Wi-Fi provisioning/reconnection/clearing, SNTP/date rollover, screen/input behavior, NVS migration/reboot retention and available internal heap during networking.

Full image: 1,839,472 bytes, SHA-256 `c2582a4634740dcf8e9484026000e18c65c9a9895142153a423930802612670f`. Matching ELF SHA-256: `2f257dc98151ea5ceb0c42814e58bc239c0e6919761eb4820cf736f204a82e8b`. Embedded version: `4d29fba-dirty`; SDK `v5.5.3`. Archive: `build/firmware/c2582a4634740dcf8e9484026000e18c65c9a9895142153a423930802612670f/`, verified with `tools/archive_firmware.py verify`. Application source is committed as `4d29fba`; the build began before the commit and retains its configured version suffix. Generated firmware and debug files stay outside Git.

Networking reference inspected: upstream `demo/blufi-provisioning` at `9c039cc5127f22072afa83bedb7fa3d8efe635ad`. Only stack/lifecycle patterns were consulted; this app uses its own WPA2 hotspot and local Web setup, with Bluetooth disabled.

With user approval, archive `c2582a4634740dcf8e9484026000e18c65c9a9895142153a423930802612670f` was flashed on COM3 at `0x0`, `0x8000` and `0x10000`, preserving the NVS region. A deliberate reset and bounded startup capture confirmed version `4d29fba-dirty`, SDK `v5.5.3` and ELF prefix `2f257dc98`. Warnings appeared at about 5.5 and 10.5 seconds. Matching-ELF decoding placed sampled PCs in LVGL draw dispatch and label drawing; this establishes rendering activity during idle-task starvation, not the root cause. Application/LVGL initialization completed; runtime acceptance failed. The serial port was closed; raw logs remain local under ignored `work/`. No repair or replacement firmware was flashed.

## Manual watering validation (2026-09-30)

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0.
- Host tests: PASS — complete suite, watering timing/input guards, retry/duplicate prevention, record format and actual NVS-worker fault injection.
- LVGL rendering and fonts: PASS — 18 snapshots, including flight/pouring/return, saving, failure, completion, maximum count and unavailable records; actual Chinese glyph checks plus negative control.
- Device tests: NOT RUN — watering interaction acceptance awaits user observations; segmented flashing and startup checks passed on COM3.
- Unverified: physical animation, new text readability, real NVS save/reboot recovery and interrupted-write behavior.

Full image: 1,160,512 bytes, SHA-256 `a9ac0d58f4456be2dd8a606d2ad67f125908d1f838b73a29908b542515602c76`. Matching ELF SHA-256: `ab291f3774db0db4ae6195abe3b0cc7aee7f5e057dbe9a77c1ce258da7907f5b`. Embedded version: `0125d4a-dirty`; SDK `v5.5.3`. Archive: `build/firmware/a9ac0d58f4456be2dd8a606d2ad67f125908d1f838b73a29908b542515602c76/`, independently verified with `tools/archive_firmware.py verify`. Generated firmware and debug files stay outside Git.

With explicit user approval, the verified archive components were flashed on COM3 at `0x0`, `0x8000` and `0x10000`; all three write hashes passed. The NVS region (`0x9000`–`0xEFFF`) was outside the erased/write ranges. A deliberate reset followed by a 15-second startup capture confirmed ESP-IDF 5.5.3, version `0125d4a-dirty`, matching embedded ELF hash prefix `ab291f377`, application/LVGL initialization and no observed panic, watchdog or storage error. The serial port was closed after capture. Raw logs remain local under ignored `work/`.

The existing 8 MB partition layout is unchanged. Segmented flashing preserves the NVS partition. Flashing the merged image at `0x0` may reset saved watering counts. Identify the device and obtain approval before either operation.
