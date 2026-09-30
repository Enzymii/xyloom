<p align="right"><a href="passport-v0.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# AI Passport Prototype v0: development and architecture

## Goal and baseline

Build an offline companion world. Milestone A proves character scale, Chinese text, physical input, and continuous camera movement on the device. Mobile Web, backend, BLE synchronization, AI chat, travel photos, and network provisioning are outside this iteration.

Upstream: https://github.com/FoloToy/ai-passport . Baseline `0b9e4c81ee4421c0bac39ca3561d65a8285acd4a`. Retain BSP and partitions. Startup uses the new application; baseline demo sources remain as references but are not linked.

## Frozen v0 contract

| Area | Contract |
| --- | --- |
| Space | Garden left, house right, door on the house's left |
| Controls | Left/right tap selects local focus; OK interacts; house left hold enters garden, garden right hold returns home |
| Other inputs | No special OK hold, double-click, or chord action; fast independent taps remain taps |
| Character | HOME_IDLE / HOME_SLEEP / OUT; OUT removes the character from both locations |
| Plant | Orange tulip: seed, sprout, bud, bloom at provisional growth 0–1 / 2–4 / 5–8 / 9+ |
| Drinking | Select can and OK; at most four records per local day, one watering credit per record |
| Watering | Select plant and OK; consume credit and increase growth on completed action; OUT does not block magic watering |
| Time | DAY 07:00–19:00; NIGHT otherwise, warm house lamps and dark window, moonlit garden |
| Outings | Persist a generated daily departure/return plan; player cannot command departure or return |
| Standby | Restore current scene; consume the complete first wake gesture |
| Persistence | Later versioned NVS stores growth, drinks, credits, date, outing plan, last scene |

Unfrozen policies: unused-credit rollover/cap (proposed cap four without silent loss), setting a trustworthy offline clock, sleep hours, outing probability/duration. A never uses uptime as a calendar date.

## Milestone A boundary

Implemented source: boot to house, transparent chibi character, OK welcome dialog and OK dismiss; a 700 ms left hold starts a 650 ms smoothstep pan through the doorway; right hold returns. Each scene retains two focus positions. World is 528×320 with a 240×320 viewport; garden camera x=0, house x=288, doorway between.

A cartoon orange tulip and a coral-red watering can identify interaction positions; dialogs do not credit drinks or growth. Domain enums reserve HOME_SLEEP, OUT, day/night, and four plant stages; automatic schedules, night art, and sleeping pose are deferred. OUT rendering hides the character. Standby switches off the backlight after 60 seconds and retains RAM state; it is not deep sleep or power-loss persistence.

Assets: reference-derived `mochun-idle-reference-v3.png`, continuous `world-day-master-v2.png`. Visual revision enlarges the head, softens hair curls with a rounder, compact two-head-tall silhouette, extends the interior floor beneath the feet, and uses a small ground highlight instead of a selection rectangle. Current character stands; seated/sleeping poses follow later. Important content avoids the official 30 px rounded screen mask.

## Responsibilities and concurrency

```text
BSP: display, sole ADC owner, LVGL render task
  -> calibrated millivolts, 10 ms polling
main/main.c: hardware adapter, loop, backlight, LVGL locking
  -> raw key
main/passport/input.c: 30 ms debounce, 700 ms hold, one event, wake consumption
  -> semantic event
main/passport/world.c: focus, dialog, transition, standby, pure state
  -> read-only world
main/passport/view.c: LVGL objects, artwork, ground-level focus markers, Chinese dialog
```

Input and world compile independently of ESP-IDF/LVGL. The application loop is the sole state writer; all drawing access holds `bsp_lvgl_lock()`. No network tasks or second ADC1 unit. BSP initializes its button task with no application callback; application reads its shared calibrated voltage API to avoid the demo's 500 ms hold/double-click delay. Provisional physical mapping is BSP UP→LEFT and DOWN→RIGHT, pending device confirmation.

Transitions clear dialog, consume input without replay, and commit scene only at the endpoint. Ladder jumps cannot create a second gesture without release. ADC failure cancels the gesture until stable release. Unsigned time deltas handle millisecond wraparound.

## Rendering, fonts, and memory

Background RGB565: 528×320×2 = 337,920 bytes. Character RGB565A8: 150×200×3 = 90,000 bytes. Tulip RGB565A8: 55×65×3 = 10,725 bytes. Watering can RGB565A8: 60×45×3 = 8,100 bytes. Total 446,745 bytes of const Flash image data. RGB565A8 is an RGB plane followed by an alpha plane. Reuse the BSP partial DMA buffer; no world-sized framebuffer or PSRAM. `tools/prepare_passport_images.py` uses Pillow for reproducible conversion; source/generated images live under `assets/images/passport/`.

Chinese font is a SIL OFL Noto Sans SC subset: weight 500, 18 px, 4 bpp, uncompressed, generated with `lv_font_conv@1.5.3`. Source, OFL, and generated C are in `assets/fonts/passport/`. Widgets explicitly select the font. All labels and punctuation must be covered; changing text requires regeneration and coverage validation. The full TTF is not linked.

Measure minimum free internal heap, largest free block, Flash size, frame duration, and Chinese readability on hardware. Retain the baseline 24 KB LVGL pool until measurement justifies changes; no unmeasured frame-rate or memory guarantee.

## Following increments

1. A hardware acceptance: boot, Chinese glyphs, key mapping, camera direction, corners and scale; pause feature expansion until accepted.
2. B data: versioned NVS, no automatic erase of incompatible data, date rollover and clock-confidence policy.
3. B interaction: daily drinks, watering credits, four plant stages, transactional updates and magic-can animation.
4. C life: day/night art, sleep, persisted randomized outing plan, offline elapsed-time recovery.
5. C power: low-power strategy, cold-boot restore, development-only debug parameters.

Persist completed business events, never animation frames. Save watering credit consumption and growth atomically to prevent power-loss duplication. Preserve records when wall-clock time is untrusted; reboot must not reset daily limits.

## Validation and device acceptance

`tests/test_passport.c` covers bounce, short/long exclusivity, undefined OK hold, wake consumption, transition gating, invalid ADC, timer wrap, camera monotonicity/endpoints, per-scene focus, standby restore, and OUT dialog. It runs in `tools/validate.sh --static`; complete gate is `tools/validate.sh`, requiring ESP-IDF 5.5.3 for firmware.

Device checklist:

- Cold boot shows house, character and character focus, with no baseline menu.
- OK displays the exact Chinese welcome without missing glyphs; another OK dismisses it.
- Release before 700 ms changes focus only; holding left at least 700 ms transitions once, with no extra tap on release.
- Pan left through the doorway into garden and right home; repeat 50 times without artifacts or reset.
- Input during animation is not replayed; garden standby followed by OK wakes only, next OK interacts.
- Record key mapping, actual visual behavior, and resource logs. Desktop rendering/build success does not replace hardware testing.

See the delivery record for actual check results. Flash only after identifying and verifying an exact firmware and obtaining user authorization; USB detection alone does not trigger flashing.

## Reproduce the desktop rendering check

Use LVGL 9.5.0 sources and a host C/C++ compiler; no browser, server, or mobile application is involved.

```sh
cmake -S tests/passport_render -B build/host-render -DLVGL_SOURCE=/path/to/lvgl-9.5.0
cmake --build build/host-render -j8
cd build/host-render
./passport_render
```

The test uses a 24 KB LVGL allocator, a 240×20 RGB565 partial display buffer, the actual application view and generated assets. It verifies glyph descriptors plus a negative control and saves house/dialog/door/garden/garden-can/house-room/status/status-unavailable/status-sleep PPM images for inspection. This proves host rendering, not physical display color, speed, or power consumption. The target chip still requires the ESP-IDF firmware gate.

## Status page

In the house, tap left/right to select Status (the second focus), then press OK. OK closes the page. Existing long-press navigation also closes it and enters the garden; standby closes it and waking restores the scene. The page displays battery percentage, uptime in hours/minutes, and the current HOME_IDLE / HOME_SLEEP / OUT state. Uptime is elapsed time since boot, not a calendar clock. Autonomous state scheduling remains deferred.

A dedicated application worker initializes the existing BSP battery gauge and polls every 10 seconds. It publishes only an atomic percentage; the main loop remains the sole world-state writer. Unavailable readings display --%; the same percentage appears at the screen's top right. No battery estimate or charging status is fabricated. Worker lifetime equals the application lifetime; no screen is deleted. Host rendering covers valid/missing battery values and all character statuses. Real gauge readings remain a device check.
