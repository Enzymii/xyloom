<p align="right"><a href="roadmap.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Cottage roadmap

Checkpoint: 2026-10-09. Continue the existing checkout on `feature/cottage-watering`; inspected HEAD `d849764`. Existing edits to the paired validation records are preserved. Product direction lives in [vision](vision.md); implementation details live in [architecture](passport-v0.md).

## Actual state

| Area | Source status | Acceptance status |
| --- | --- | --- |
| Milestone A | House startup, chibi art, welcome dialog, local focus, 700 ms hold / 650 ms camera pan, return and standby implemented | Prior simulator evidence exists; current-image full UI/input acceptance remains open |
| Watering and growth | Flying can, atomic save/retry, cups, daily limit, stages 0/4/20/56 and four small header drops implemented; plant descriptions replace numeric progress | Host/render checks recorded; physical interaction and persistence still need acceptance |
| Offline continuity | Watering before sync, persisted 24-hour powered cycles, conservative calendar handoff implemented | Host regressions added; power-cut/reboot and sync need physical acceptance |
| Networking | Temporary setup hotspot, remembered Wi-Fi and SNTP date; optional RAM-only simulator auto-connect profile; BLE disabled | Simulator worker regression added; real provisioning/reconnect/calendar boundaries remain unverified |
| Day/night and walks | Night artwork, seated sleep, daily walk and remembered scene time implemented | Host logic/storage/render checks; exact-image simulator and device acceptance pending |
| Cloud and future candidates | Deferred | Outside current scope |

The validation history contains a watchdog failure for image `c2582a46...` and a later limited 45-second startup pass for `fab28dc2...`. The latter is not full Milestone A acceptance. Simulator interaction for the stage-percentage revision was PARTIAL: house rendered, button responses were not observed. Do not erase older failures or label all images PASS.

## Next small batches, in order

1. **A1 — Restore reproducible simulator interaction.** Reproduce short press and explicit press/release holds using the unmodified archived image and matching ELF. Separate simulator input/runtime faults from application faults. Keep any simulator-only diagnostic work outside shipped firmware. Acceptance: welcome open/close, both focus positions, doorway midpoint, both endpoints, and wake gesture consumption demonstrated on the exact image. If blocked, retain PARTIAL and use host rendering as complementary evidence.
2. **A2 — Close physical Milestone A.** After explicit authorization for the exact device/image, verify cold boot, readable character/dialog, three-button mapping, both transitions, idle backlight and first-wake consumption. Observe bounded startup logs for watchdogs; startup alone does not pass UI acceptance. Record exact image identity and observations in the existing validation records.
3. **A3 — Protect work already present.** Verify real Wi-Fi setup/reconnect, offline cold boot, powered-cycle rollover, conservative synchronization, fifth-cup growth cap, stage boundaries, bloom recording and reboot persistence. Investigate failures with targeted fixes before adding features. This accepts existing functionality, rather than expanding Milestone A.

The 2026-10-08 follow-up fixes a stale input guard after a stable release during a transition and adds raw-input journey regressions. Simulator interaction remains PARTIAL; physical acceptance is still pending. Each batch should contain one reproducible problem, one narrow fix if needed, focused regression checks and a recorded acceptance result.

## Local MVP batches (source implemented; acceptance pending)

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
