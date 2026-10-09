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
| Drinking | Every successful watering records one cup; only the first four per local powered cycle or trusted UTC+8 day advance growth |
| Watering | Current increment: select the can and press OK to water manually, then atomically save cups, quota and growth; OUT does not block magic watering |
| Time | DAY 07:00–19:00; NIGHT otherwise, warm house lamps and dark window, moonlit garden |
| Outings | Persist a generated daily departure/return plan; player cannot command departure or return |
| Standby | Restore current scene; consume the complete first wake gesture |
| Persistence | Versioned NVS stores growth, total/today cups, quota, date and powered-runtime checkpoint; the outing plan is derived from remembered scene time; camera position remains volatile |

Growth allowance does not roll over: unused daily points are not stored credits. Growth never decays. UTC+8 calendar dates come from online SNTP; uptime is never treated as a date. Sleep hours and outing schedules are implemented under the life contract below.

## Milestone A boundary

Implemented source: boot to house, transparent chibi character, OK welcome dialog and OK dismiss; a 700 ms left hold starts a 650 ms smoothstep pan through the doorway; right hold returns. Each scene retains two focus positions. World is 528×320 with a 240×320 viewport; garden camera x=0, house x=288, doorway between.

The Milestone A cartoon orange tulip and coral-red watering can initially displayed informational dialogs; the current watering increment is described below. HOME_SLEEP, OUT, day/night, four plant stages and automatic life schedules are implemented; the life contract is below. OUT rendering hides the character. Standby switches off the backlight after 60 seconds and retains RAM state; it is not deep sleep. Cups, growth, daily quota and date now survive power loss; camera position remains volatile; character scheduling resumes from remembered scene time.

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

Background RGB565: 528×320×2 = 337,920 bytes. Character RGB565A8: 150×200×3 = 90,000 bytes. Four tulip stages, each RGB565A8: 55×65×3 = 10,725 bytes. Watering can RGB565A8: 60×45×3 = 8,100 bytes. With the night background and sleep sprite, total const image data is 906,840 bytes. RGB565A8 is an RGB plane followed by an alpha plane. Reuse the BSP partial DMA buffer; no world-sized framebuffer or PSRAM. `tools/prepare_passport_images.py` uses Pillow for reproducible conversion; source/generated images live under `assets/images/passport/`.

Chinese font is a SIL OFL Noto Sans SC subset: weight 500, 18 px, 4 bpp, uncompressed, generated with `lv_font_conv@1.5.3`. Source, OFL, and generated C are in `assets/fonts/passport/`. Widgets explicitly select the font. All labels and punctuation must be covered; changing text requires regeneration and coverage validation. The full TTF is not linked.

Measure minimum free internal heap, largest free block, Flash size, frame duration, and Chinese readability on hardware. Retain the baseline 24 KB LVGL pool until measurement justifies changes; no unmeasured frame-rate or memory guarantee.

## Following increments

1. A hardware acceptance: boot, Chinese glyphs, key mapping, camera direction, corners and scale; include the current manual-watering increment in device acceptance.
2. Current data acceptance: versioned NVS migration, daily rollover, reboot quota and clock confidence.
3. Current interaction acceptance: daily cups, four plant stages, atomic updates and magic-can animation.
4. C life: day/night art, sleep, remembered scene time and deterministic daily outing plan, offline elapsed-time recovery.
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

In the house, tap left/right to select Status (the second focus), then press OK. OK closes the page. Existing long-press navigation also closes it and enters the garden; standby closes it and waking restores the scene. The page displays battery percentage, uptime in hours/minutes, and the current HOME_IDLE / HOME_SLEEP / OUT state. Uptime is elapsed time since boot, not a calendar clock. Autonomous state scheduling follows the life contract below.

A dedicated application worker initializes the existing BSP battery gauge and polls every 10 seconds. It publishes only an atomic percentage; the main loop remains the sole world-state writer. Unavailable readings display --%; the same percentage appears at the screen's top right. No battery estimate or charging status is fabricated. Worker lifetime equals the application lifetime; no screen is deleted. Host rendering covers valid/missing battery values and all character statuses. Real gauge readings remain a device check.


## Daily cups, growth and Wi-Fi

Every successful watering records one cup. The first four cups per quota cycle advance growth; later cups still count toward today and lifetime totals. Thresholds are 4 for sprout, 20 for bud and 56 for bloom: fourteen full growth allowances from a new seed. Growth never decays; bloom remains visible while drinking continues. The garden keeps four 10-by-14-pixel water drops in the top header at x=88/102/116/130, y=28, between the object label and Wi-Fi indicator. They have no backing panel and hide outside the garden or during a transition. Plant inspection uses short stage-specific descriptions; watering completion gives one brief acknowledgement. Cup counts, growth percentages, progress bars and timing/refresh rules are not shown in the garden. Background cup recording and the four-point allowance remain intact. No next plant or collection system is implemented.

The worker saves a snapshot frozen when watering starts; an operation crossing midnight belongs to its starting day. Failed saves retry exactly that snapshot without replaying animation or counting twice. The application loop alone updates world state after commit.

Without a trusted date, watering is available immediately. A local cycle refreshes four growth slots after 86,400 seconds of accumulated powered runtime, including backlight standby. Power-off time is excluded. Cups beyond four still count, and unused slots never accumulate. The local cycle operates silently; it does not add a timing instruction to the garden. An undated local bucket is carried into a bridge bucket on first synchronization; only the next forward calendar date refreshes it. On reboot, a previously dated bucket instead enters an awaiting-sync mode: background checkpoints preserve its date evidence. If no offline cup or local-cycle refresh occurs, synchronization to a later date clears used drops and cycle cups while preserving total cups and plant growth; same-day synchronization retains the allowance. An actual offline drink or local refresh makes the bucket undated, retaining the conservative bridge. Backward dates fall back to local timing without granting quota. A synchronized clock continues through a network outage while powered.

The current `cottage` / `watering` record is 40 bytes: `XYWC` at 0–3, version 4 at 4, growth/quota at 5–6, clock mode at 7, little-endian uint32 lifetime cups/cycle cups/last YYYYMMDD at 8–19, little-endian uint64 accumulated runtime at 20–27, uint32 local-cycle phase at 28–31, and reserved zeros at 32–39. Modes are local (0), calendar (1), bridge (2) and awaiting sync (3). Awaiting mode requires a valid saved date and retains the powered-cycle phase. `growth_record.c` validates the data. Version 3 preserves all fields; already-undated version-3 local records stay conservative because their cup dates cannot be recovered. Version 2 preserves plant, cups, date and quota with an empty runtime checkpoint; version 1 preserves lifetime totals and starts a new plant because it lacks historical growth. Migration commits with the next cup or clock checkpoint. Missing records start empty; incompatible data/NVS errors remain intact and disable watering without erasure. Wi-Fi credentials use a separate SDK namespace. Older firmware cannot read version 4; a downgrade must preserve it rather than reset records.

The world accumulates monotonic elapsed time and requests a worker checkpoint every 60 seconds when not animating/saving and at least 200 ms after input, plus quota/date-mode boundaries. Watering includes the elapsed checkpoint in its atomic transaction. Acknowledgement subtracts only the frozen elapsed time; animation and retry time remain pending. Failed writes retain the snapshot and expose OK retry, including after standby/wake; input cannot replace it. Clock-only commits do not show watering completion or postpone standby. The worker validates clock-only and drink transitions separately. Abrupt power loss can discard time since the last successful checkpoint, normally under a minute plus the pending operation; long holds or storage failure can extend this window. It cannot reset the saved quota or duplicate a saved cup. This conservative loss delays the next allowance rather than advancing it.


The Wi-Fi icon beside the battery is green with an IP connection and muted while disconnected. In the house's Status page, left/right selects Return, Set Wi-Fi or Clear Wi-Fi. OK executes the selection. Clearing requires a second OK; left/right cancels. Only Wi-Fi settings are cleared, never cups/growth. Long-hold navigation remains available.

Set Wi-Fi opens the WPA2 hotspot `Xyloom-Setup` with a fresh eight-character password displayed on-screen. Connect a phone, stay connected despite its no-Internet warning, then visit `http://192.168.4.1`. Select a scanned 2.4 GHz network or enter its name and an 8–63-byte password. The device remembers one network in ESP-IDF's local storage; reconfiguration replaces it. Application logs/source contain no credentials. Bluetooth and the BLUFI mini program are not used. The phone page has its own minimal UI.

The HTTP task scans and receives credentials; a separate network worker owns connection actions and never accesses LVGL. Setup closes five seconds after IP connection or after five minutes. Wrong credentials or unavailable networks can be corrected on the page or by reopening setup. Reconnection tries every ten seconds for five attempts, then every sixty seconds. SNTP uses `ntp.aliyun.com`; the UTC+8 date is trusted only after a plausible synchronization in the current boot. After sync, time continues during an outage while powered; reboot requires a fresh sync only for calendar dating, not for interaction. Status shows local timing when the date is unavailable. Forward midnight rollover and clock-mode changes use the same atomic checkpoint path as local cycles.

Host tests cover the 14-day minimum, daily cap/excess cups, leap/month/year boundaries, thresholds, bloom recording, unknown/backward clocks, 24-hour boundaries, multiple cycles, reboot phase/quota, conservative sync, version-1/2 migration, clock-only commits, transaction retries and credential form parsing. Device acceptance must cover setup discovery, scan/manual network selection, wrong password, saved reconnection, reconfiguration, credential clearing, SNTP, midnight, readability and internal heap during concurrent networking/UI.

The setup endpoint accepts the AP address in IPv4 and IPv4-mapped IPv6 form, matching ESP-IDF’s dual-stack HTTP listener, and rejects other destination addresses. The view compares visible state before updating LVGL; unchanged scenes do not refresh continuously, while watering, transitions, water drops, plant stages and Status uptime still update when their displayed state changes. Hidden cup changes and growth within a stage do not redraw a stationary garden.

## Simulator networking

`CONFIG_COTTAGE_SIMULATOR_NETWORK` is disabled by default. The optional
`sdkconfig.simulator.defaults` profile connects automatically to the open
`Emulator Host Bridge` access point. The simulator forwards traffic through its
local server and the computer's current internet connection; no home-network
password is needed. Wi-Fi configuration uses RAM and never reads, replaces or
clears physical-device credentials. Watering records still use normal NVS
storage and clock rules.
The profile routes application logs through UART0, which the browser runtime
captures, instead of the physical-device USB-Serial-JTAG console. UART0's
physical pins conflict with this board's backlight; this is another reason
never to flash the simulator profile to hardware.

Build this development profile with `./tools/validate.sh --simulator`. It runs
host checks and the firmware gate with both defaults files, retains a verified
archive, and copies the image to `build/Cottage-simulator-full.bin`. It does not
replace the normal `build/FoloToy-AI-Passport-full.bin`. Upload the simulator
image to the local simulator, never flash it to a physical device. The normal
complete gate remains `./tools/validate.sh`.

In Cottage's Status page, select Clear Wi-Fi and confirm with OK to disconnect
and stop automatic retries for this session. Select Set up Wi-Fi and press OK
to reconnect to the virtual access point without starting the physical setup
portal. Restarting the simulator profile reconnects. Loss of internet upstream
may leave Wi-Fi connected while SNTP fails: the bridge indicator alone does not
prove a trusted date. Confirm the Cottage Status date and UART synchronization
event, plus the Wi-Fi inspector's DNS and UDP traffic. A synchronized clock can
continue after disconnect; a truly unsynchronized cold boot needs the bridge
blocked before boot. Real provisioning, radio behavior and BLE remain device
checks.

`tests/test_passport_network.c` executes the actual simulator-profile worker
with stubbed driver/task events: RAM-only configuration, initial connection,
trusted SNTP, forget without reconnect, explicit reconnect, transient
configuration retry and startup failure cleanup. It asserts that flash
credential reads and restore calls never occur.

## Manual watering foundation (superseded record format)

The following is the historical count-only increment, superseded by the daily-growth format and UI above.

The initial watering increment saved completed counts only. The current behavior and record format are specified below. The can travels for 450 ms, tilts and pours for 900 ms, then returns for 450 ms: 1,800 ms total. Watering also works while matsujun is OUT. Animation and saving consume input; held keys require release before another action. Input is not queued, counted again, or used for scene transitions or standby during the operation.

After the animation, the storage worker saves the target count. Only a successful save updates the world's count and shows completion. A failed save retains the prior count; OK retries the same target without replaying the animation or crediting twice. Reboot restores only saved completions. Power loss can leave the old or committed new value, never a partial record. The uint32 limit rejects further increments rather than wrapping.

The dedicated `main/passport/storage.c` worker owns NVS. The main loop sends snapshots and receives results through queues, remaining the sole world-state writer. The existing NVS partition stores a 12-byte blob in namespace `cottage`, key `watering`: bytes 0–3 are `XYWC`, byte 4 is version 1, bytes 5–7 are reserved zeros, and bytes 8–11 are the little-endian uint32 count. `watering_record.c` handles portable encoding/decoding. A missing record starts at zero. NVS initialization errors, incompatible formats or versions show unavailable storage and preserve existing data without automatic erase/reset. Worker startup failure also disables watering and shows an error.

This increment does not implement a trusted calendar, daily limits, drink conversion, plant-growth animation, or scene recovery. The four stage enums remain a future interface; the flower still uses the existing fixed illustration. The watering counter is not a completed growth system.

Host tests cover timer wrap, OUT watering, animation/save guards, duplicate prevention, save retries, count limits, record formats, and fault injection into the actual NVS worker. Rendering adds travel, pouring, return, saving, failure, completion and counter snapshots, with complete Chinese glyph coverage. Device checks: can selection starts watering, flower selection inspects it, animation is not clipped, held keys do not trigger extra actions, completed counts survive reboot, interrupted saves retain valid records, and existing transitions/standby still work.

A stable release during a transition re-arms the next fresh gesture immediately after the transition; a key still held, an invalid ADC reading, or an unconfirmed release remains suppressed. The raw-input journey regression covers dialogs, both pans, watering save failure/retry and wake consumption.

## Local life rhythm (2026-10-09)

`life.c` provides pure day/night, sleep and outing logic. Day is 07:00–18:59 UTC+8; night is 19:00–06:59. The character rests from 22:00 to 06:59, with a dedicated transparent seated sprite and a quiet sleep dialog. Each local date has one deterministic 30-minute outing, starting between 10:00 and 16:59. It is derived from the epoch day, so a reboot does not reroll that day's plan. OUT hides the character in either camera view; watering and its atomic transaction remain available.

The network publishes trusted UTC seconds only after accepted SNTP. Main advances a remembered scene clock using monotonic powered time when offline. A device without any remembered/trusted time starts in daytime HOME_IDLE, with no guessed outing. A separate 16-byte versioned/checksummed `cottage/life` NVS record remembers scene time. The storage worker checkpoints every 15 minutes and at night/sleep/outing boundaries, with retries spaced at least five seconds. Power-off time is excluded; sudden loss can delay the scene clock by the unsaved interval. Scene-time storage is separate from the versioned watering quota record described above. Invalid/unknown life records are preserved and disable scene checkpoints while live operation and watering remain usable.

Tests cover exact light/sleep/outing boundaries, reboot plan stability, offline return, corrupt records and real storage-worker failures. Host LVGL snapshots include both light modes, sleep/dialog, OUT and watering during OUT. Physical timing, colors and power loss still require acceptance.

### Local simulator input adapter

The local simulator's synthetic click timing was not reliable for Cottage's release-based input in quiet scenes. Explicit ADC level taps demonstrated welcome/dismiss and focus on the unchanged archived firmware. `tools/cottage_simulator_input.patch` provides a simulator-side adapter for filenames containing `Cottage`: each synthetic tap becomes a queued 30 ms wall-time press plus a 30 ms release gap. Double-click sends two complete taps. Long holds still use actual press/release, and other firmware names retain upstream gestures. Apply this patch from the simulator source root with `git apply /absolute/path/to/tools/cottage_simulator_input.patch`; it changes `public/app.js`, not firmware or BSP. Keep Cottage in the local image filename to select this adapter. Wall-time pacing is a local development aid, not a hardware timing measurement; current-image acceptance results are recorded separately.

### Simulator transport switch

`tools/cottage_simulator_offline.patch` adds a local test-only transport switch to the simulator, retained across firmware uploads and hard restart in that page. Apply it from the simulator source root after the input patch. The switch closes the host network bridge and discards outgoing guest frames; restoring it reconnects that bridge. It never changes the computer's network settings, credentials or firmware clock. The emulator may still provide a virtual association and DHCP lease, so a green Wi-Fi link icon is not proof of Internet access. Verify the absence of accepted SNTP and the offline date field. A page reload resets the switch to online. This patch changes only simulator JavaScript, not the device firmware.
