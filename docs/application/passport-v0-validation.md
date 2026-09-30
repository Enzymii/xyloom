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
