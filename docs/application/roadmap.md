<p align="right"><a href="roadmap.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Cottage roadmap

Checkpoint: 2026-10-09. Local MVP implementation is committed and pushed as `2658014` on `feature/cottage-watering`. Initial physical startup and owner-reported basic UI acceptance pass; the remaining device checks below stay open. Product direction lives in [vision](vision.md); implementation details live in [architecture](passport-v0.md).

## Actual state

| Area | Source status | Acceptance status |
| --- | --- | --- |
| Milestone A | House startup, chibi art, welcome dialog, local focus, 700 ms hold / 650 ms camera pan, return and standby implemented | Exact-image simulator dialog/focus/pan/wake observed; physical startup and owner-reported navigation pass; device standby/wake and distinct midpoint remain open |
| Watering and growth | Flying can, atomic save/retry, cups, daily limit, stages 0/4/20/56 and four small header drops implemented; plant descriptions replace numeric progress | Host/render/simulator checks pass; owner reports normal physical watering; device persistence and boundaries remain open |
| Offline continuity | Watering before sync, persisted 24-hour powered cycles, conservative calendar handoff implemented | Host and unsynchronized simulator checks pass; physical offline cold boot, power loss and rollover remain open |
| Networking | Temporary setup hotspot, remembered Wi-Fi and SNTP date; optional RAM-only simulator auto-connect profile; BLE disabled | Host/simulator checks pass; real retained Wi-Fi connection and SNTP observed; new provisioning/reconnect and calendar boundaries remain open |
| Day/night and walks | Night artwork, seated sleep, daily walk and remembered scene time implemented | Host/render and exact-image simulator fixtures pass; physical time/outing boundaries and sustained operation remain open |
| Cloud and future candidates | Deferred | Outside current scope |

The validation history contains a watchdog failure for image `c2582a46...` and a later limited 45-second startup pass for `fab28dc2...`. The latter is not full Milestone A acceptance. Simulator interaction for the stage-percentage revision was PARTIAL: house rendered, button responses were not observed. Do not erase older failures or label all images PASS.

## Initial acceptance plan (historical)

1. **A1 — Restore reproducible simulator interaction.** Reproduce short press and explicit press/release holds using the unmodified archived image and matching ELF. Separate simulator input/runtime faults from application faults. Keep any simulator-only diagnostic work outside shipped firmware. Acceptance: welcome open/close, both focus positions, doorway midpoint, both endpoints, and wake gesture consumption demonstrated on the exact image. If blocked, retain PARTIAL and use host rendering as complementary evidence.
2. **A2 — Close physical Milestone A.** After explicit authorization for the exact device/image, verify cold boot, readable character/dialog, three-button mapping, both transitions, idle backlight and first-wake consumption. Observe bounded startup logs for watchdogs; startup alone does not pass UI acceptance. Record exact image identity and observations in the existing validation records.
3. **A3 — Protect work already present.** Verify real Wi-Fi setup/reconnect, offline cold boot, powered-cycle rollover, conservative synchronization, fifth-cup growth cap, stage boundaries, bloom recording and reboot persistence. Investigate failures with targeted fixes before adding features. This accepts existing functionality, rather than expanding Milestone A.

The 2026-10-08 follow-up fixes a stale input guard after a stable release during a transition and adds raw-input journey regressions. Simulator interaction remains PARTIAL; physical acceptance is still pending. Each batch should contain one reproducible problem, one narrow fix if needed, focused regression checks and a recorded acceptance result.

## Local MVP batches (source implemented; full device acceptance pending)

- **B1:** day/night visual switching with offline remembered time. Verify 07:00/19:00 boundaries and Chinese glyph coverage.
- **B2:** HOME_SLEEP pose and 22:00–07:00 sleep schedule; protect input and watering behavior.
- **B3:** remembered time and a reproducible daily departure/return plan, OUT absence in both scenes and continued magic watering; test reboot/time changes and daily rollover.
- **Later:** lightweight cloud synchronization only after local storage and acceptance are stable.

Journal, travel postcards, collection book, monthly letters/yearly reports and customizable characters remain unscheduled candidates. Nookling is terminology, not a current task.

## Authorized MVP completion batch: 2026-10-09

The owner requested the remaining local Cottage MVP features and simulator verification before next-morning acceptance. This extends source implementation through B1/B2/B3 while keeping A's physical acceptance open. Day/night artwork, 22:00–07:00 sleep, one reproducible half-hour daily outing, remembered offline scene time, separate versioned NVS storage and host regressions are implemented. The existing quiet cultivation UI, watering record and production provisioning stay intact. The concrete life rules are documented in the architecture.

Next acceptance order: exact-image simulator buttons and transitions; Status disconnect/reconnect; online-to-offline watering; unsynchronized cold boot; light/sleep/outing boundary fixtures and reboot recovery; then the owner's physical Milestone A and local MVP checks. The candidate list above remains deferred. No cloud, Journal, postcards, collection, reports or customizable characters are added. Final image identities and actual results belong in the existing validation record.

## Morning handoff checkpoint

Local B1/B2/B3 are implemented and covered by host tests and bounded simulator flows on the exact final simulator image. Both normal and simulator complete build gates pass. The validation record contains image identities, synthetic-fixture distinctions and remaining checks. A1 now has observed dialog/focus/pan/wake behavior with a simulator-side input adapter; the door midpoint remains host-rendered evidence. A2/A3 device acceptance stays open. Continue with user acceptance and normal-profile hardware testing before extending the roadmap.

## Initial physical acceptance: 2026-10-09

The owner authorized commit, push and flashing. The normal archived image was flashed to the connected ESP32-C3 after verifying the existing partition table, with NVS and PHY excluded from writes. A bounded 45-second observation confirmed the matching ELF, application startup, retained real Wi-Fi connection and SNTP, with one boot and no crash markers. After being asked to enter the garden, water and return home, the owner reported normal display and controls. This is basic device acceptance; the full matrix remains open. Artifact identity and limits are in the [validation record](passport-v0-validation.md).

Next small checks, in order: physical offline cold boot and reboot persistence; fifth-cup growth cap and stage boundaries; standby/backlight and first-wake consumption; then real provisioning/reconnect and time/quota/outing rollover. Fix any reproducible failure before adding features. Cloud and future candidates remain deferred.

## Reboot quota correction (pre-flash checkpoint): 2026-10-09

The owner reported old used drops on a physical next-day boot. Source now preserves a dated bucket while waiting for SNTP, clears it on a later trusted date, and keeps same-day/offline anti-duplication. Version-4 records migrate existing versions without erasure; already-undated old buckets stay conservative. Host logic, actual storage-worker retry, and LVGL header checks pass; the normal complete build gate passes. This new artifact still needs physical acceptance. The earlier code commit and device PASS above remain historical; this correction is uncommitted and unflashed. Exact image identities and remaining checks are in the [validation record](passport-v0-validation.md).

Next: authorize this artifact's segmented device flash after connecting USB, then verify next-day synchronization and same-day persistence before continuing other acceptance tasks. The quiet cultivation UI and future-feature deferrals remain unchanged.

## Correction flashed and records reset: 2026-10-09

The owner authorized overwriting incorrect records, so the independently verified normal correction image was written as a complete refresh to COM3. NVS/PHY and saved Wi-Fi were reset as explained before writing. Transfer verification passed; a bounded 45-second startup matched the ELF with one boot, no crash markers and no watering-storage errors. The owner confirmed empty header drops and seed stage in the garden. The serial port is released. The owner then authorized commit and push. Exact identity and bounded acceptance are in the [validation record](passport-v0-validation.md).

Next acceptance: configure Wi-Fi/SNTP again, water once and check same-day reboot persistence; then verify next-day refresh and offline power-loss/runtime boundaries. Visible reset is accepted; automatic daily behavior still needs hardware evidence. Future-feature deferrals remain unchanged.
