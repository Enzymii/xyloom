<p align="right"><a href="passport-v0.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# AI Passport Prototype v0: development and architecture

## Goal and baseline

Build a companion world. Milestone A establishes character scale, Chinese text, input and camera movement. The current increment adds daily drinking, plant growth and Wi-Fi date synchronization; a local phone Web page is used only for provisioning. Backend, BLE synchronization, chat and travel photos remain deferred.

Upstream: https://github.com/FoloToy/ai-passport . Baseline `0b9e4c81ee4421c0bac39ca3561d65a8285acd4a`. Retain BSP and partitions. Startup uses the new application; baseline demo sources remain as references but are not linked.

## v0 direction and current watering controls

| Area | Contract |
| --- | --- |
| Space | Garden left, house right, door on the house's left |
| Controls | Left/right tap selects local focus; OK interacts; house left hold enters garden, garden right hold returns home |
| Other inputs | No special OK hold, double-click, or chord action; fast independent taps remain taps |
| Character | HOME_IDLE / HOME_SLEEP / OUT; OUT removes the character from both locations |
| Plant | Orange tulip: seed, sprout, bud, bloom at growth 0–3 / 4–19 / 20–55 / 56 |
| Drinking | Every successful watering records one cup; only the first four per trusted UTC+8 day advance growth |
| Watering | Current increment: select the can and press OK to water manually, then atomically save cups, quota and growth; OUT does not block magic watering |
| Time | DAY 07:00–19:00; NIGHT otherwise, warm house lamps and dark window, moonlit garden |
| Outings | Persist a generated daily departure/return plan; player cannot command departure or return |
| Standby | Restore current scene; consume the complete first wake gesture |
| Persistence | Versioned NVS stores growth, total/today cups, daily quota and date; outing plan and last scene remain deferred |

Growth allowance does not roll over: unused daily points are not stored credits. Growth never decays. UTC+8 calendar dates come from online SNTP; uptime is never treated as a date. Sleep hours and outing schedules remain deferred.

## Milestone A boundary

Implemented source: boot to house, transparent chibi character, OK welcome dialog and OK dismiss; a 700 ms left hold starts a 650 ms smoothstep pan through the doorway; right hold returns. Each scene retains two focus positions. World is 528×320 with a 240×320 viewport; garden camera x=0, house x=288, doorway between.

The Milestone A cartoon orange tulip and coral-red watering can initially displayed informational dialogs; the current watering increment is described below. Domain enums reserve HOME_SLEEP, OUT, day/night, and four plant stages; automatic schedules, night art, and sleeping pose are deferred. OUT rendering hides the character. Standby switches off the backlight after 60 seconds and retains RAM state; it is not deep sleep. Cups, growth, daily quota and date now survive power loss; scenes and character scheduling remain volatile.

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

Input and world compile independently of ESP-IDF/LVGL. The application loop is the sole state writer; all drawing access holds `bsp_lvgl_lock()`. Storage and networking have separate workers; there is no second ADC1 unit. BSP initializes its button task with no application callback; application reads its shared calibrated voltage API to avoid the demo's 500 ms hold/double-click delay. Provisional physical mapping is BSP UP→LEFT and DOWN→RIGHT, pending device confirmation.

Transitions clear dialog, consume input without replay, and commit scene only at the endpoint. Ladder jumps cannot create a second gesture without release. ADC failure cancels the gesture until stable release. Unsigned time deltas handle millisecond wraparound.

## Rendering, fonts, and memory

Background RGB565: 528×320×2 = 337,920 bytes. Character RGB565A8: 150×200×3 = 90,000 bytes. Four tulip stages, each RGB565A8: 55×65×3 = 10,725 bytes. Watering can RGB565A8: 60×45×3 = 8,100 bytes. Total 478,920 bytes of const Flash image data. RGB565A8 is an RGB plane followed by an alpha plane. Reuse the BSP partial DMA buffer; no world-sized framebuffer or PSRAM. `tools/prepare_passport_images.py` uses Pillow for reproducible conversion; source/generated images live under `assets/images/passport/`.

Chinese font is a SIL OFL Noto Sans SC subset: weight 500, 18 px, 4 bpp, uncompressed, generated with `lv_font_conv@1.5.3`. Source, OFL, and generated C are in `assets/fonts/passport/`. Widgets explicitly select the font. All labels and punctuation must be covered; changing text requires regeneration and coverage validation. The full TTF is not linked.

Measure minimum free internal heap, largest free block, Flash size, frame duration, and Chinese readability on hardware. Retain the baseline 24 KB LVGL pool until measurement justifies changes; no unmeasured frame-rate or memory guarantee.

## Following increments

1. A hardware acceptance: boot, Chinese glyphs, key mapping, camera direction, corners and scale; include the current manual-watering increment in device acceptance.
2. Current data acceptance: versioned NVS migration, daily rollover, reboot quota and clock confidence.
3. Current interaction acceptance: daily cups, four plant stages, atomic updates and magic-can animation.
4. C life: day/night art, sleep, persisted randomized outing plan, offline elapsed-time recovery.
5. C power: low-power strategy, cold-boot restore, development-only debug parameters.

Persist completed business events, never animation frames. Save cup counts, daily quota and growth atomically to prevent power-loss duplication. Preserve records when wall-clock time is untrusted; reboot must not reset daily limits.

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


## Daily cups, growth and Wi-Fi

Every successful watering records one cup. The first four cups per trusted UTC+8 calendar day advance growth; later cups still count toward today and lifetime totals. Thresholds are 4 for sprout, 20 for bud and 56 for bloom: at least 14 days from a new seed. Growth never decays; bloom remains visible while drinking continues. The garden shows today's cups, four daily slots and a growth bar. Plant inspection shows its stage, the whole completion percentage within that stage and today’s cups. The bar uses the same stage percentage. Seed spans 0–4, sprout 4–20 and bud 20–56: each newly entered stage starts at 0%, while bloom remains at 100%. Percentages are rounded down; the cumulative x/56 value is not displayed. No lifetime cups or completed-day totals are displayed. No next plant or collection system is implemented.

The worker saves a snapshot frozen when watering starts; an operation crossing midnight belongs to its starting day. Failed saves retry exactly that snapshot without replaying animation or counting twice. The application loop alone updates world state after commit.

The current `cottage` / `watering` record is 24 bytes: `XYWC` at 0–3, version 2 at 4, growth/daily quota at 5–6, zero at 7, little-endian uint32 lifetime cups/today cups/YYYYMMDD date at 8–19, zeros at 20–23. `growth_record.c` validates the data. Version-1 records preserve lifetime totals but start a new plant at zero because historical dates and quotas were not recorded. Migration commits with the next completed cup. Missing records start empty; incompatible data/NVS errors are preserved and disable watering without automatic erasure. Wi-Fi's SDK manages its credential namespace separately from the application record worker.

The Wi-Fi icon beside the battery is green with an IP connection and muted while disconnected. In the house's Status page, left/right selects Return, Set Wi-Fi or Clear Wi-Fi. OK executes the selection. Clearing requires a second OK; left/right cancels. Only Wi-Fi settings are cleared, never cups/growth. Long-hold navigation remains available.

Set Wi-Fi opens the WPA2 hotspot `Xyloom-Setup` with a fresh eight-character password displayed on-screen. Connect a phone, stay connected despite its no-Internet warning, then visit `http://192.168.4.1`. Select a scanned 2.4 GHz network or enter its name and an 8–63-byte password. The device remembers one network in ESP-IDF's local storage; reconfiguration replaces it. Application logs/source contain no credentials. Bluetooth and the BLUFI mini program are not used. The phone page has its own minimal UI.

The HTTP task scans and receives credentials; a separate network worker owns connection actions and never accesses LVGL. Setup closes five seconds after IP connection or after five minutes. Wrong credentials or unavailable networks can be corrected on the page or by reopening setup. Reconnection tries every ten seconds for five attempts, then every sixty seconds. SNTP uses `ntp.aliyun.com`; the UTC+8 date is trusted only after a plausible synchronization in the current boot. After sync, time continues during an outage while powered; reboot requires a fresh sync. Unknown or backward dates block new watering without resetting quota. Status distinguishes connection from synchronized date. Displayed today/quota roll over at midnight; persistence changes only on a completed cup.

Host tests cover the 14-day minimum, daily cap/excess cups, leap/month/year boundaries, thresholds, bloom recording, unknown/backward clocks, reboot quota, migration, transaction retries and credential form parsing. Device acceptance must cover setup discovery, scan/manual network selection, wrong password, saved reconnection, reconfiguration, credential clearing, SNTP, midnight, readability and internal heap during concurrent networking/UI.

The setup endpoint accepts the AP address in IPv4 and IPv4-mapped IPv6 form, matching ESP-IDF’s dual-stack HTTP listener, and rejects other destination addresses. The view compares visible state before updating LVGL; unchanged scenes do not refresh continuously, while watering, transitions, counters and Status uptime still update when their displayed state changes.

## Manual watering foundation (superseded record format)

The following is the historical count-only increment, superseded by the daily-growth format and UI above.

The initial watering increment saved completed counts only. The current behavior and record format are specified below. The can travels for 450 ms, tilts and pours for 900 ms, then returns for 450 ms: 1,800 ms total. Watering also works while matsujun is OUT. Animation and saving consume input; held keys require release before another action. Input is not queued, counted again, or used for scene transitions or standby during the operation.

After the animation, the storage worker saves the target count. Only a successful save updates the world's count and shows completion. A failed save retains the prior count; OK retries the same target without replaying the animation or crediting twice. Reboot restores only saved completions. Power loss can leave the old or committed new value, never a partial record. The uint32 limit rejects further increments rather than wrapping.

The dedicated `main/passport/storage.c` worker owns NVS. The main loop sends snapshots and receives results through queues, remaining the sole world-state writer. The existing NVS partition stores a 12-byte blob in namespace `cottage`, key `watering`: bytes 0–3 are `XYWC`, byte 4 is version 1, bytes 5–7 are reserved zeros, and bytes 8–11 are the little-endian uint32 count. `watering_record.c` handles portable encoding/decoding. A missing record starts at zero. NVS initialization errors, incompatible formats or versions show unavailable storage and preserve existing data without automatic erase/reset. Worker startup failure also disables watering and shows an error.

This increment does not implement a trusted calendar, daily limits, drink conversion, plant-growth animation, or scene recovery. The four stage enums remain a future interface; the flower still uses the existing fixed illustration. The watering counter is not a completed growth system.

Host tests cover timer wrap, OUT watering, animation/save guards, duplicate prevention, save retries, count limits, record formats, and fault injection into the actual NVS worker. Rendering adds travel, pouring, return, saving, failure, completion and counter snapshots, with complete Chinese glyph coverage. Device checks: can selection starts watering, flower selection inspects it, animation is not clipped, held keys do not trigger extra actions, completed counts survive reboot, interrupted saves retain valid records, and existing transitions/standby still work.
