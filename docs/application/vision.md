<p align="right"><a href="vision.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Xyloom / Cottage vision

Xyloom is a small companion world; Cottage is its current FoloToy AI Passport app. The Chinese brand and default character names are recorded in the Chinese version. The default character is matsujun. Nookling is reserved for a future character system.

## Product contract

- Local-first: scenes, interaction and durable personal progress belong on the device. Optional lightweight cloud synchronization follows later.
- A continuous space: garden on the left, house on the right, door on the house's left. Camera movement preserves this geography.
- Three buttons: left/right taps select local objects; OK interacts and dismisses Galgame-style dialogs. Hold left from home to enter the garden; hold right from the garden to return.
- A chibi companion with HOME_IDLE, HOME_SLEEP and OUT states. Autonomous walks create a sense of independent life; watering remains available while the character is away.
- Quiet cultivation: four small water drops sit directly in the garden header. Growth is felt through plant illustrations and gentle text, with cup counts, percentages and refresh rules kept in the background.
- Drinking and an orange tulip provide a gentle daily ritual. Four stages, cumulative growth, no decay, and at most four growth points per quota cycle. Bloom remains while drinking records continue.
- Day from 07:00 to 19:00, night otherwise. Standby consumes the first wake gesture and restores the scene.

## Existing implementation and offline contract

The existing manual action records a cup when watering completes. Cups are not capped at four; growth is. Thresholds are 0, 4, 20 and 56, requiring fourteen full growth allowances to bloom. Do not replace this implemented transaction with separate drinking credits without an explicit product decision.

Offline watering, cup recording and growth are available before Wi-Fi/SNTP. Without a trusted date, the allowance refreshes after each 24 hours of accumulated powered runtime. Reboot preserves the saved cycle and used allowance; powered-off time does not count. This local cycle stays in the background; the garden does not display a calendar claim or refresh rule. First synchronization preserves used allowance and cups; the next trusted calendar date restores natural-day refresh. Backward dates fall back to the local cycle without resetting allowance. This implementation retains old plant/cup records; physical acceptance remains pending.

## Current focus

Close Milestone A: boot to the chibi companion, OK welcome dialog, and a smooth leftward pan through the doorway to the garden. Existing growth and networking work stays intact. Source implementation and hardware acceptance are separate; see the [roadmap](roadmap.md) and [validation history](passport-v0-validation.md).

Simulator-first UI iteration uses unmodified firmware where possible, with host LVGL rendering as a complementary check. Simulator results do not establish physical BLE, Wi-Fi, battery, backlight or input behavior. Real Wi-Fi requires a device; BLE is not enabled in Cottage.

## Future candidates

Journal, travel postcards, a collection book, monthly letters/yearly reports and customizable characters are candidates only. They have no current implementation commitment. Cloud synchronization is later work; no backend or character-system expansion belongs in Milestone A.

The authorized 2026-10-09 batch extends source work through the local MVP life rhythm: day/night, sleep and daily walks. Milestone A physical acceptance remains open; future candidates remain deferred.
