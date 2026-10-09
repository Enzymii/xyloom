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
- Device tests: NOT RUN — runtime acceptance incomplete; segmented flash verification passed, but USB disconnected during startup capture.
- Unverified: physical percentage/bar readability and interactions.

Full image: 1,840,176 bytes, SHA-256 `6b575e6c5705f4033be15325d4de4630e40afa4b716d3159ca9789a2be25fd33`. ELF SHA-256: `3b86971ffd041bfd6a1b9437b07d52dfba92c26f5f7e66fa91da3a5f4aabe11c`. Embedded version: `feb6de2-dirty`; SDK `v5.5.3`. Verified archive: `build/firmware/6b575e6c5705f4033be15325d4de4630e40afa4b716d3159ca9789a2be25fd33/`. With explicit user approval, the verified archive was segmented-flashed on COM3 at `0x0`, `0x8000` and `0x10000`; all three write hashes passed and NVS was outside the erased/write ranges. A deliberate reset started a 45-second capture, but the USB serial connection disappeared before completion. Read-only re-enumeration found no Passport USB device or COM3. The capture failed before saving its buffer, so this attempt supplies no verified startup-log evidence; the serial handle closed on error. Reconnect the powered device with a data-capable cable before repeating the bounded startup check. No additional flash or erasure was attempted.

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

## Core interaction regression (2026-10-08)

A stable release during a blocked transition could leave the input guard active when a fresh press arrived on the first unblocked sample. The targeted regression failed on the previous implementation and passes after clearing the guard from the already-confirmed release before replacing the input candidate. Held keys, ADC faults and releases shorter than debounce remain suppressed.

Raw-input journey coverage uses the application's 10 ms sampling order: welcome/dismiss, both camera pans, can selection, missing-date guard, watering save failure/retry without duplicate cups, focus retention, house wake hold and garden wake OK without unintended actions. Existing persistence fault-injection tests remain in place.

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0; merged image and matching debug archive independently verified.
- Host tests: PASS — complete suite plus focused raw-input/transition-release regressions.
- LVGL rendering and fonts: PASS — 32 snapshots, actual glyph coverage and stationary redraw checks; core scene/dialog/doorway/watering frames visually inspected.
- Device tests: NOT RUN — read-only Windows enumeration did not identify a Passport; no port was opened or firmware flashed.
- Simulator: PARTIAL — unmodified archived `fab28dc2...` and `a9ac0d58...` boot to the house but did not respond to OK or explicit held UP. The original `831f7d1d...` welcome dialog responded, and the built-in Demo responded to DOWN.
- Unverified: simulator root cause, exact-new-image interaction, physical button mapping/transitions, backlight/wake and NVS recovery.

The `fab28dc2...` archive was independently verified. Its startup ELF prefix `24fe0a87c` matched; sampled PC `0x40385b9e` decodes in that ELF to `esp_cpu_wait_for_intr`. This is a sampled wait location, not proof of a specific timer fault. Simulator-only diagnostic changes were not added to the application or BSP. Wi-Fi behavior, fonts, assets and stored-record format are unchanged.

Full image: 1,840,176 bytes; SHA-256 `dbcdec658617fd8275f653db23042d6082b4e7825483ea5a93a9baab0c968eb1`. Matching ELF: `d5dcb2a314ccf8927bdffa472af8cb20dacdfe08bce6dab7fdceb978a36d7172`. Embedded version: `d849764-dirty`; SDK `v5.5.3`. Archive: `build/firmware/dbcdec658617fd8275f653db23042d6082b4e7825483ea5a93a9baab0c968eb1/`. Components at `0x0`, `0x8000`, `0x10000` retain the existing partition layout; compatible segmented flashing avoids NVS, whereas merged-image flashing from `0x0` can reset it. No flash was performed.

New-image simulator check: PARTIAL. The unmodified `dbcdec65...` merged image displayed the Cottage house. UART reported `d849764-dirty`, SDK `v5.5.3` and matching ELF prefix `d5dcb2a31`. OK and an explicitly held/released UP still produced no observed dialog or pan. This iteration fixes the independently reproduced input guard; it does not claim to repair the simulator runtime issue. The original `831f7d1d...` control also completed the leftward pan to the garden.

## 2026-10-08: local-first watering before synchronization

Offline watering no longer waits for Wi-Fi/SNTP. A persisted powered-runtime cycle supplies four growth points every 24 hours; additional cups count without growth. Power-off time is excluded. First synchronization retains used quota/cups in a bridge bucket, with natural-day rollover starting on the next forward date. Existing version-1/2 records migrate without erasing plant or cup data. The UI uses cycle cups, four slots and an explicit refresh rule; unavailable calendar dates display local timing.

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0; merged image and matching debug archive independently verified. No physical device was flashed.
- Host tests: PASS — growth/record, world/raw-input journey and actual NVS-worker regressions, including 24-hour boundaries, multiple cycles, restart quota/phase, clock rollback, conservative synchronization, clock-only validation, write failure/retry and standby retry preservation.
- LVGL rendering/font: PASS — 36 snapshots, including offline quota, plant details, local status, refreshed slots and calendar bridge; generated glyph descriptors and stationary refresh regression checked. Host images do not establish device rendering.
- Device tests: NOT RUN — read-only serial enumeration found only COM1, the computer's communication port; no Passport device was detected or opened.
- Unverified: physical offline cold boot, saved quota/phase after a power cut, real SNTP handoff/midnight, screen/input, concurrent Wi-Fi/UI heap, and existing simulator interaction limitation.

The 60-second checkpoint can wait for an operation or held input; failed storage requires retry. Abrupt power loss loses only elapsed time since the last successful checkpoint, delaying the next allowance. Saved cups, growth and quota remain atomic. Migration to version 3 is incompatible with older firmware readers; rollback must retain the stored record rather than erase it.

Verified full image: 1,844,048 bytes; SHA-256 `1afdbbd42d5df6572e29b57b1cb7676d2bb8cbd69765a27da228e850efbdea44`. Matching ELF SHA-256: `e4ecf46d9c229c1d9fdf5491baa74a8db50497c0f31857baecc3889d90b86ce8`. Version `d849764-dirty`, SDK `v5.5.3`. Archive: `build/firmware/1afdbbd42d5df6572e29b57b1cb7676d2bb8cbd69765a27da228e850efbdea44/`. App is 1,778,512 bytes; existing 8 MB partition layout retained. Compatible segmented flashing at `0x0`, `0x8000`, `0x10000` excludes NVS; a merged flash at `0x0` may reset NVS.

Simulator: PARTIAL on this exact image. Startup reports ESP-IDF `v5.5.3`, `d849764-dirty`, ELF prefix `e4ecf46d9`; the house and companion render. An OK click and explicit UP hold/release produced event-log entries but no observed dialog or garden transition. This reproduces the prior limitation and does not validate offline interactions. No simulator-specific patch was added to firmware.

Files updated in this working iteration (including the preceding Milestone A guard fix):

- Application: `main/main.c`; `main/passport/input.c`; `growth.c`, `growth.h`, `growth_record.c`, `growth_record.h`, `storage.c`, `storage.h`, `view.c`, `world.c`, `world.h` under `main/passport/`.
- Font: `assets/fonts/passport/passport_font_18.c`, `tools/prepare_passport_font.ps1`.
- Tests: `tests/test_passport.c`, `tests/test_passport_growth.c`, `tests/test_passport_storage.c`, `tests/passport_render/render.c`.
- Entry documents: `README.md`, `README.zh_CN.md`, `docs/README.md`, `docs/README.zh_CN.md`.
- Product documents: `docs/application/vision.md`, `vision.zh_CN.md`, `roadmap.md`, `roadmap.zh_CN.md`.
- Architecture and evidence: `docs/application/passport-v0.md`, `passport-v0.zh_CN.md`, `passport-v0-validation.md`, `passport-v0-validation.zh_CN.md`.

Next: authorize exact-image device testing after connecting the Passport, then accept offline cold boot, five cups, reboot retention, quota-boundary behavior, real synchronization and the existing navigation/wake flow. Do not add future features before these checks. No commit or push was performed.

## 2026-10-08: quiet cultivation and miniature header drops

The garden uses four 10-by-14-pixel water drops in its top header, directly between the object label and Wi-Fi indicator. The white hydration panel, displayed cup counts, growth bar, percentages and cycle-refresh instructions were removed. Plant inspection now uses short stage descriptions, and watering completion gives one brief response. Cup recording, quotas, offline runtime, migration and saved growth are unchanged.

- Build: PASS — complete `tools/validate.sh`, ESP-IDF 5.5.3, exit 0; merged image and matching debug archive independently verified.
- Host tests: PASS — repository/static and existing state/storage suites. LVGL rendering/font: PASS, 36 current snapshots; hidden cup and within-stage progress changes stay idle, while retained drops and stage changes still redraw. Added glyphs are present in the generated font.
- Device tests: NOT RUN — no flashing authorization or detected Passport.
- Unverified: miniature drops and text readability on the physical display, physical interactions and the previous simulator input limitation. This iteration has not been exercised in the simulator.

Updated files: `main/passport/view.c`, `tests/passport_render/render.c`, `tools/prepare_passport_font.ps1`, `assets/fonts/passport/passport_font_18.c`, and the paired README, vision, roadmap, architecture and validation documents. Next: validate the exact new image, then offer physical display testing without changing the local-first rules.

Verified full image: 1,844,400 bytes; SHA-256 `25185440ab08d0089cdfa4ec98a0a7435ee7d27edfb5c72dcad146f64fd8e2bf`. Matching ELF SHA-256: `941399ae44e042a78c51fb659937c70016e58d47f57b01448bc3ebbd9e6e8b5a`. Version `d849764-dirty`, SDK `v5.5.3`. Archive: `build/firmware/25185440ab08d0089cdfa4ec98a0a7435ee7d27edfb5c72dcad146f64fd8e2bf/`. Application: 1,778,864 bytes. The existing partition layout remains compatible with segmented flashing at `0x0`, `0x8000`, `0x10000`, excluding NVS. Merged-image flashing at `0x0` may reset NVS. No device was flashed and no commit/push was performed. Read-only serial enumeration still found only COM1.

## Simulator network profile — 2026-10-09

The simulator profile automatically connects to the open `Emulator Host Bridge`
using the computer's network. Wi-Fi driver credential NVS is disabled and
configuration is RAM-only; physical-device credentials are neither loaded nor
overwritten. Application watering NVS is unchanged. Status clear stops retries;
Status setup restores the virtual connection. UART0 application logging is
enabled only by the simulator defaults; normal firmware retains USB console
and physical setup behavior. Never flash the simulator profile to hardware.

- Build: PASS — normal complete gate and simulator complete gate, ESP-IDF 5.5.3;
  both merged-image/debug archives independently verified, exported hashes match.
- Host tests: PASS — full static gate, actual network-worker connect/SNTP/forget/
  reconnect/repeated setup/transient retry/failure cleanup tests. Simulator bridge:
  PASS, 12 tests. Direct host NTP received a valid 48-byte response.
- Simulator networking: PASS on the final image below. UART reports ELF prefix
  `80374595f`, Wi-Fi credential NVS disabled, connection to the virtual AP,
  IP `192.168.4.2`, and `Network time synchronized` at boot elapsed 6071 ms.
  The earlier network-only image `d245cd33...` demonstrated DNS and NTP traffic
  and a green application Wi-Fi indicator, but its USB application logs were
  unavailable in the UART inspector; it is superseded by the UART profile.
- Device tests: NOT RUN — no flash authorization; only COM1 detected.
- Unverified: simulator UI input/Status clear and reconnect, unsynchronized
  offline cold boot, physical Wi-Fi provisioning/reconnect and display/input.
  Network acceptance does not close Milestone A input acceptance.

On the final image, a DOWN click and a controlled 300 ms wall-clock DOWN
press/release produced inspector input events but no observed focus change.
The key was released. UI acceptance remains PARTIAL; the cause is not established.

Simulator full image: 1,797,024 bytes, SHA-256
`a209707affb7ad9d6ef80456b9d00a65b16b74d8db029ec6fe396f1c7c33a681`;
application 1,731,488 bytes; matching ELF SHA-256
`80374595f52f8c5bffafa52a2d6a70e55ae4b5f47beb8cd270d70e0dbabe2374`.
Archive: `build/firmware/a209707affb7ad9d6ef80456b9d00a65b16b74d8db029ec6fe396f1c7c33a681/`.
Normal full image: 1,844,672 bytes, SHA-256
`3fd7ee7d3e98edd81b6b8b0db425100f6eb52b1f863452a26ac9194ef392bcf0`;
application 1,779,136 bytes; matching ELF SHA-256
`b1f680fb4c5b098910cdf01bc7cdc7698c6bffe99246ab20bd5337bda266b27d`.
Archive: `build/firmware/3fd7ee7d3e98edd81b6b8b0db425100f6eb52b1f863452a26ac9194ef392bcf0/`.
Both versions are `d849764-dirty`, SDK `v5.5.3`. No flash, commit or push occurred.

Updated files: `main/passport/network.c`, `main/Kconfig.projbuild`,
`sdkconfig.simulator.defaults`, `tools/validate.sh`, `tests/test_passport_network.c`,
`tests/passport_network_stubs/`, and the paired README, architecture, roadmap,
build/test and validation documents. Next: reproduce simulator input on this
exact image, exercise Status disconnect/reconnect, then block networking before
boot to validate the unsynchronized offline path. Keep real-device acceptance
separate and use normal firmware after explicit flashing authorization.

## Local MVP completion and exact-image acceptance — 2026-10-09

The owner authorized completing the existing local Cottage MVP before morning acceptance. No new project, cloud feature, flash, commit or push was created. Source work continues on `feature/cottage-watering`, HEAD `d849764`, with pre-existing changes preserved.

Implemented: a moonlit continuous world, seated sleep sprite, 07:00/19:00 light boundaries, 22:00–07:00 rest and sleep dialog, one reproducible 30-minute daily outing, absent character/marker during OUT, continued magic watering, and powered-time continuation of a remembered scene clock. Separate checksummed/versioned life storage runs through the existing worker; watering v3 and the quiet four-drop UI remain intact. Total generated constant image payload is 906,840 bytes in Flash; this is not a measured hardware memory result.

- Build: PASS — complete normal and simulator gates under ESP-IDF 5.5.3, exit 0; both merged-image/debug archives independently verified. The builds used the original working checkout and original toolchain. An abandoned temporary native-build copy was not used.
- Host tests: PASS — complete static gate, pure light/sleep/outing boundary tests, storage-worker life persistence/error/retry/unknown-record preservation and watering during OUT. Actual LVGL rendering/font checks pass with 42 snapshots, including the new sleep glyphs and idle redraw regressions. Network bridge regression suite passes all 12 tests.
- Simulator: PASS for the bounded flows listed below; Milestone A physical acceptance remains open. These are emulator results, not hardware results.
- Device tests: NOT RUN — the owner postponed physical acceptance until morning. No firmware was flashed.
- Unverified: physical display colors/glyph readability, sleep sprite sizing, button/ADC timing, backlight, heap/stack headroom, provisioning/BLE/real Wi-Fi, real power-loss behavior and complete device acceptance. Exact time/quota/outing boundaries were covered by host tests rather than a 24-hour emulator soak. The door midpoint has a host-rendered snapshot; a distinct midpoint was not captured in this simulator session.

### Final image identities

| Profile | Full image SHA-256 | Full bytes / app bytes | Matching ELF SHA-256 |
| --- | --- | --- | --- |
| Simulator | `c02ff6266eb4f74fd69d67d40391cd480b275c9d47895c403b7f65917c7a7ac9` | 2227616 / 2162080 | `348384846c16cceb1f563ed8011fbba32c77c38b9c6dd979fb50e8cf2e85614b` |
| Device | `98a46c48653bc0833e65caac53d96d318158dc151c9baf2a5a33cd2d952c5bcd` | 2274944 / 2209408 | `7a7ddf8f34fb756c0a4bda11733f2f9b53075e1a31f524224385bbb73e7c22c2` |

Both versions are `d849764-dirty`, SDK `v5.5.3`. Matching archives are `build/firmware/<full-image-sha256>/`. The simulator is RAM Wi-Fi plus UART0; never flash that profile to hardware. Use normal firmware for device acceptance. The existing NVS/PHY/factory layout is unchanged. Segmented writes from the device archive at `0x0`, `0x8000`, `0x10000` exclude NVS; a full merged-image write at `0x0` can reset stored records. No full-chip erase is required.

### Observed simulator flows

On the unmodified final simulator image, boot logs identify ELF prefix `348384846`, an IP address and accepted SNTP. Real network time displayed night and HOME_SLEEP automatically. The sleep dialog was readable. Status displayed the date and changed the link state on confirmed clear. Offline garden watering finished successfully, used one header drop, and a hard restart retained three remaining drops. Restart also restored the development profile's virtual network as designed.

After pausing only the host bridge before boot, the same unmodified image had no accepted SNTP or remembered life record: daytime HOME_IDLE remained usable and watering finished. Status showed offline time. Restoring the bridge and selecting Status setup after clear reconnected, obtained an IP and accepted SNTP (`717664` ms in that run); Status then displayed 2026-10-09 and sleep. Welcome open/dismiss, house and garden focus, both pan endpoints and first-wake gesture consumption were observed. Standard on-screen OK/DOWN clicks worked with the local input adapter after allowing the simulator's 350 ms click/double-click dispatch window; explicit ADC-level probes also worked. All held keys were released at handoff.

The earlier interaction limitation was reproduced on the unchanged archived network image, then explicit level taps/holds produced responses. No general firmware input defect was established from that limitation. `tools/cottage_simulator_input.patch` provides a local wall-paced ADC tap adapter, while `tools/cottage_simulator_offline.patch` adds a test transport switch. Neither patch changes firmware. The virtual radio/DHCP may stay connected while Internet transport is paused; its green link icon is not Internet reachability evidence.

Synthetic test NVS records additionally exercised offline 19:01 night/HOME_IDLE with sprout, 23:00 sleep with bud, and the daily outing with bloom. OUT hid the character and ground marker, and watering during OUT finished and consumed a drop. These fixture images changed only `0x9000–0xEFFF` of the final simulator base, retaining identical application, bootloader and partition bytes. Their SHA-256 identities are respectively `c65b310f1887d00292dc6f3005b94f3869f0a54b2ac45fdcdf9013dad1c9ab21`, `e1992e5ebfec9992162a9c0b68acf9840e6f1b22d230773e496bf3bdad897af1`, and `4a080f9d793601b89ef37046fd9b1508fcca5e06d59b6f60a646db857c289c8a`. They are test fixtures, not delivery firmware. An earlier fixture CSV incorrectly used `data,binary` for file paths, causing `ESP_ERR_NVS_INVALID_LENGTH`; correcting it to `file,binary` restored valid records. That failed fixture is not counted as production acceptance. Normal-profile fixture interaction was not accepted. The browser was returned to the unmodified simulator image with the bridge online.

### Updated files and next acceptance

- Application: `main/main.c`, `main/CMakeLists.txt`, `main/passport/life.c`, `life.h`, `storage.c`, `storage.h`, `network.c`, `network.h`, `world.c`, `world.h`, `view.c`.
- Assets/tools: night and sleep master/target PNGs, `assets/images/passport/passport_images.c`, `assets/fonts/passport/passport_font_18.c`, `tools/prepare_passport_images.py`, `tools/prepare_passport_font.ps1`, both simulator patches.
- Checks: `tests/test_passport_life.c`, `tests/test_passport_storage.c`, `tests/test_passport.c`, `tests/passport_render/render.c`, `tools/validate.sh`.
- Paired documentation: root README, assets README, vision, roadmap, architecture and this validation record. Earlier offline/network batch files remain part of the accumulated working changes.

Next: morning simulator acceptance, then explicitly authorized physical testing with the normal image: cold boot, three keys/wake, both pans, offline five cups/reboot, real Wi-Fi setup/reconnect, life scenes and stored-data preservation. Cloud sync, Journal, postcards, collection, reports and custom characters remain deferred.

## Initial physical acceptance — 2026-10-09

The owner explicitly authorized commit, push and flashing. Implementation commit `2658014` was pushed to `feature/cottage-watering`. The previously validated normal archive was used without rebuilding after the commit: its embedded version remains `d849764-dirty`, SDK `v5.5.3`. This identifies the pre-commit build of the implemented source; it is not a newly built `2658014` artifact.

Device: COM3, ESP32-C3 revision v1.1, 8 MB Flash, USB Serial/JTAG. The existing partition table was read and matched the target exactly: NVS `0x9000` / `0x6000`, PHY `0xF000` / `0x1000`, factory `0x10000` / `0x7F0000`. Only bootloader at `0x0`, partition table at `0x8000` and application at `0x10000` were written; all three transfer hashes verified. No full-chip erase, NVS write, PHY write or firmware readback was performed. Stored Wi-Fi settings remained usable.

The archive's full-image SHA-256 is `98a46c48653bc0833e65caac53d96d318158dc151c9baf2a5a33cd2d952c5bcd`; matching ELF SHA-256 is `7a7ddf8f34fb756c0a4bda11733f2f9b53075e1a31f524224385bbb73e7c22c2`. Archive verification passed before segmented flashing. The full merged image itself was not written.

A bounded 45-second serial observation after reset matched ELF prefix `7a7ddf8f3`, saw one boot, BSP/application startup, retained real Wi-Fi connection, and SNTP synchronization at approximately 5.9 seconds. No panic, abort, watchdog, brownout, backtrace or reboot loop was detected. Startup free heap was 189,860 bytes and largest block 114,688 bytes before networking; these do not establish steady-state or peak memory headroom. The battery IC was detected and its configured profile matched. A late-window disconnect/setup-AP transition was logged; its cause was not established and it does not pass reconnection acceptance. The serial port was released afterward. Raw logs and personal network identifiers remain local and untracked.

After being asked to enter the garden, water and return home, the owner reported normal display and controls. This is owner-reported basic interaction acceptance, not direct visual inspection or a complete device test matrix.

- Build: PASS — previously completed normal and simulator full gates under ESP-IDF 5.5.3; the flashed normal artifact was independently verified. No firmware source changed during this acceptance recording.
- Host tests: PASS — previous complete static/host and LVGL rendering checks; documentation-only follow-up is checked again before commit.
- Device tests: PASS — limited to matching-artifact startup, retained real Wi-Fi/SNTP, and owner-reported basic display/navigation/watering/return. Full Milestone A and local MVP hardware acceptance remain open.
- Unverified: physical offline cold boot, watering/quota persistence after reboot or sudden power loss, fifth-cup cap and growth boundaries, standby/backlight/first-wake behavior, new provisioning and reconnect, clock/quota/outing rollover, detailed font/color/sleep-sprite inspection and sustained memory/stack headroom.

Updated files: this validation record and the roadmap, both language pairs. Next: physical offline cold boot and persistence, then quota/stage limits and standby/wake; keep cloud and future candidates deferred.
