# Project Milestone Checkpoints

> **Audit Rule:** A checkpoint item is marked complete only after its corresponding Pull Request has been reviewed, verified on hardware or build check, and merged into `dev`/`main`.

## 1. Phase Milestone Summary Table (09 Nov – 24 Nov 2026)

| Milestone Tag | Phase Window | Internal Cutoff | Status | Merge Commit Hash | Verification Summary |
|---|---|---|---|---|---|
| `m1-hardware` | **Phase 1:** 09 Nov – 12 Nov 2026 | 23:59, 12 Nov 2026 | Planned | `pending` | Repo initialized, `.ioc` configured (`Serial Wire`, `f_TIM` verified, `TIM2` 10 ms), Steps 3.1–3.4 hardware verified |
| `m2-scheduler-hal` | **Phase 2:** 13 Nov – 16 Nov 2026 | 23:59, 16 Nov 2026 | Planned | `pending` | O(1) `scheduler` + one-shot tested; `global`, `software_timer`, `traffic_light`, `button` modules verified |
| `m3-fsm` | **Phase 3:** 17 Nov – 20 Nov 2026 | 23:59, 20 Nov 2026 | Planned | `pending` | `dht20` & `lcd` I2C drivers verified; FSM diagrams complete; `fsm_automatic`, `fsm_manual`, `fsm_tuning` running |
| `v1.0-final` | **Phase 4:** 21 Nov – 24 Nov 2026 | 23:59, 24 Nov 2026 | Target Release | `pending` | Edge cases & 30-min stability test passed, PDF report exported, merged to `main` (10 days before 04 Dec deadline) |

## 2. Detailed Verification Checklist by Step (Part B Guide Alignment)

### Phase 1 (09–12 Nov) — Step 1, Step 2 & Step 3: Repo, `.ioc` & Hardware Verification
- [ ] Single GitHub repository created with `.gitignore`, `README.md`, `PROJECT_PLAN.md`, `CHECKPOINTS.md`, `CONTRIBUTIONS.md`, and `DECISIONS.md`.
- [ ] All 3 members (`Nguyen`, `Tung`, `Huy`) added as collaborators and configured university Git identity.
- [ ] STM32CubeIDE project created via **Board Selector -> NUCLEO-F103RB** with **SYS -> Debug = Serial Wire**.
- [ ] Actual timer clock `f_TIM` checked in **Clock Configuration** tab and `PSC`/`ARR` calculated so `TIM2` interrupt is exactly `10 ms`.
- [ ] Pin mapping table for `D3–D10` (`PB3, PB5, PB4, PB10, PA8, PA9, PC7, PB6`) and `I2C1` (`PB8, PB9`) configured in `.ioc`.
- [ ] `LED_PA5` blinks reliably; board can be re-flashed repeatedly without SWD lockout.
- [ ] 2-pin signal combination truth table (`00`, `01`, `10`, `11`) recorded for Traffic Light Module 1 and Module 2.
- [ ] Pressed/released logic levels verified for all 4 external buttons (`BTN_1..BTN_4`) and `USER_BTN` (`PC13`).
- [ ] `I2C1` scan (`addr << 1`) discovers both devices (`DHT20` and `LCD`) and records their 7-bit addresses in `Docs/hardware_test_notes.md`.

### Phase 2 (13–16 Nov) — Step 4 & Step 5A: Mandatory O(1) Scheduler & Core GPIO/Timer Modules
- [ ] `SCH_Update()` runs in strict **O(1)** time by decrementing only `SCH_tasks_G[0].Delay`.
- [ ] `SCH_Add_Task()` inserts tasks in sorted order and computes relative delays accurately at head, middle, and tail positions.
- [ ] `SCH_Delete_Task()` removes any arbitrary task and adds its remaining `Delay` to the next task so downstream timing is preserved.
- [ ] `SCH_Dispatch_Tasks()` processes all ready tasks (`Delay == 0`) at the head of the queue and deletes one-shot tasks (`Period = 0`).
- [ ] `global.h/.c` implements the exact shared state enum, timer indices, button indices, and duration variables from `PROJECT_PLAN.md`.
- [ ] `software_timer.h/.c` updates software timers every `10 ms` via a scheduler task and provides `setTimer()`, `clearTimer()`, and `clearAllTimers()`.
- [ ] `button.h/.c` debounces inputs every `10 ms` using a 3-register filter and provides consume-on-read `isButtonPressed(i)` and `isButtonLongPressed(i)`.
- [ ] `traffic_light.h/.c` controls both roads cleanly via `traffic_set(road, color)` and `traffic_blink_color(color)`.

### Phase 3 (17–20 Nov) — Step 5B & Step 6: I2C Drivers & State Machines
- [ ] `dht20.h/.c` reads and converts temperature and humidity periodically (every 2 seconds) without blocking.
- [ ] `lcd.h/.c` initializes and prints mode menus, countdowns, and DHT20 sensor readings over `I2C1` via `lcd_refresh_task()`.
- [ ] Zero `HAL_*` calls exist outside `main.c` and the Step 5 supporting modules.
- [ ] Formal diagrams for `fsm_automatic`, `fsm_manual`, `fsm_tuning`, and mode-switching transitions completed in `Docs/fsm_diagrams.drawio`.
- [ ] System switches cleanly across all 3 modes via `AUTO_INIT`, `MAN_INIT`, and `TUNE_INIT`; Road 1 and Road 2 are **never Green at the same time**.

### Phase 4 (21–24 Nov) — Step 7: System Testing, Report Writing & Submission
- [ ] Edge cases verified: rapid button presses, mode switch during Amber light, and invalid duration adjustment rejection (`Red != Green + Amber`).
- [ ] System runs continuously for at least 30 minutes without task array overflow, drift, or hang.
- [ ] Final PDF report `CO3009_2452859_2453391_2452401_Report.pdf` generated with submission commit hash on the cover page.
