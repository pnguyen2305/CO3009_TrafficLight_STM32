# Architectural and Implementation Decisions Log

> This document records key hardware, algorithmic, and software architecture decisions made by the team for the CO3009 Two-Road Traffic Light System project, incorporating lessons from Labs 1–4.

## D01 — 4-Phase Technical Roadmap Across 16 Days (09 Nov – 24 Nov 2026)

* **Date:** 10 Nov 2026
* **Decision:** Organize the 16-day development window (`09 Nov – 24 Nov 2026`, finishing 10 days before the `04 Dec 2026` deadline) into **4 Technical Phases (`Phase 1` through `Phase 4`)** tagged by architectural layer (`m1-hardware`, `m2-scheduler-hal`, `m3-fsm`, `v1.0-final`) rather than artificially naming 4-day intervals "Weeks".
* **Rationale:** Aligns directly with the bottom-up engineering progression in Part B0 (`Hardware -> Scheduler & GPIO Drivers -> I2C & FSMs -> System Testing & Report`) while fitting naturally into 2.5 real calendar weeks.
* **Consequence:** Every member completes 1 well-scoped work package per phase with zero ambiguity.

## D02 — STM32F103RB Target Selection, Serial Wire Debug, and `f_TIM` Clock Verification

* **Date:** 10 Nov 2026
* **Decision:** Initialize the project using **Board Selector -> NUCLEO-F103RB** (`STM32F103RB`), set **SYS -> Debug = Serial Wire**, and verify the actual APB1 Timer Clock (`f_TIM`) in the `.ioc` Clock Configuration tab before setting `TIM2` `PSC` and `ARR`.
* **Rationale:**
  1. Unlike `STM32F103C6` in Proteus (Labs 1–4), leaving Debug disabled on `STM32F103RB` disables SWD pins (`PA13`/`PA14`) after the first flash.
  2. The `NUCLEO-F103RB` board template in STM32CubeIDE defaults to a `64 MHz` PLL clock (`f_TIM = 64 MHz`) rather than the `8 MHz` internal oscillator used in Lab 2. Using `PSC = 7999, ARR = 9` at `64 MHz` would make the system tick 8x too fast (`1.25 ms`). At `64 MHz`, we set `PSC = 6399, ARR = 99` (or configure `f_TIM = 8 MHz` with `PSC = 7999, ARR = 9`) to obtain an exact `10 ms` interrupt tick.
* **Consequence:** `Nguyen` configures `.ioc` and documents the exact `f_TIM`, `PSC`, and `ARR` values in `Docs/hardware_test_notes.md`.

## D03 — Static Sorted Array with Relative Delays for Mandatory O(1) Scheduler

* **Date:** 10 Nov 2026
* **Decision:** Upgrade the Lab 4 cooperative scheduler from O(n) to **strict O(1)** in `SCH_Update()` using a static sorted array (`sTask SCH_tasks_G[SCH_MAX_TASKS]`) storing **relative delays** (`Delay` relative to the task immediately preceding it).
* **Rationale:**
  1. `SCH_Update()` inside the `TIM2` ISR only decrements `SCH_tasks_G[0].Delay` and sets `SCH_tasks_G[0].RunMe = 1` when `Delay == 0`, guaranteeing **O(1)** ISR execution.
  2. In `SCH_Dispatch_Tasks()`, whenever the head task (`SCH_tasks_G[0]`) is dispatched and removed, if the new head task also has `Delay == 0` (simultaneous wake-up), its `RunMe` flag is immediately triggered in the same pass so simultaneous tasks never slip by 1 tick.
  3. In `SCH_Delete_Task(taskID)`, if a non-tail task is removed before expiration, its remaining `Delay` is added to `SCH_tasks_G[idx + 1].Delay` before shifting array elements left.
* **Consequence:** `Huy` implements this logic in `scheduler.c` and writes the complexity proof in `Docs/scheduler_design.md`.

## D04 — Consume-on-Read Edge-Triggered Button API & 3-Register Debouncer

* **Date:** 10 Nov 2026
* **Decision:** Implement `button_reading()` (scheduled every `10 ms`) using the 3-register debounce filter (`KeyReg0`, `KeyReg1`, `KeyReg2`, and previous stable state `KeyReg3`) with **consume-on-read event flags** inside `isButtonPressed(uint8_t index)` and `isButtonLongPressed(uint8_t index)`, plus `clearAllButtonFlags()` on mode transitions.
* **Rationale:** In Lab 3, `is_button_pressed()` returned a continuous level (`1` while held), which would cause `if (isButtonPressed(0)) status = MAN_INIT;` (BTL Page 14) to register dozens of presses from a single finger push (Section B8 Symptom 4). Clearing `button_flag[index] = 0` inside `isButtonPressed(index)` guarantees each physical press triggers exactly one state transition. Holding a button for `>= 100` ticks (`1 s`) triggers `button_long_flag[index]` every `25` ticks (`250 ms`) for fast duration tuning.
* **Consequence:** `Tung` implements `button.h/.c`, and all 3 FSM authors call `isButtonPressed()` directly without extra per-button FSMs.

## D05 — Non-Blocking Throttled I2C LCD Refresh & Staggered Scheduler Offsets

* **Date:** 10 Nov 2026
* **Decision:** Encapsulate all LCD I2C writes inside `lcd_refresh_task()` scheduled every `200 ms` (`20` ticks, or immediately when `lcd_needs_update == 1`), and stagger initial task `DELAY` offsets in `main.c` (`0, 1, 3, 5, 8` ticks).
* **Rationale:** Writing 32 characters over 100 kHz I2C takes several milliseconds. Calling `lcd_send_string()` every 10 ms inside `fsm_*_run()` would block the super loop and cause LCD flicker. Following Lab 4 Section 2.3.11, staggering initial task offsets prevents `dht20_read_task` (2000 ms period) and `lcd_refresh_task` (200 ms period) from colliding on the same tick.
* **Consequence:** Smooth, flicker-free LCD updates and zero I2C bus contention.

## D06 — Traffic Light Mutual-Exclusion and Safe Mode Transition Invariant

* **Date:** 10 Nov 2026
* **Decision:** Enforce two safety invariants across `fsm_automatic`, `fsm_manual`, and `fsm_tuning`:
  1. Road 1 and Road 2 can never be `GREEN` simultaneously.
  2. Every mode switch enters through a dedicated initialization state (`AUTO_INIT`, `MAN_INIT`, `TUNE_INIT`) that calls `clearAllTimers()` and `clearAllButtonFlags()`. In Manual mode, switching the green right-of-way between roads always passes through a mandatory `2-second AMBER` state (`MAN_RED1_AMBER2` or `MAN_AMBER1_RED2`).
* **Rationale:** Directly required by Section 6.1 ("Key Focus of Assessment" — 30% weight).
* **Consequence:** `Nguyen`, `Tung`, and `Huy` follow the shared `SystemState_t` state machine contract in `global.h`.

## D07 — Duration Consistency Constraint (`Red == Green + Amber`) in Tuning Mode

* **Date:** 10 Nov 2026
* **Decision:** Edit temporary variables (`draft_red`, `draft_amber`, `draft_green` in range `1..99` seconds) during `TUNE_RED`, `TUNE_AMBER`, `TUNE_GREEN` while blinking the corresponding LED color at `2 Hz` (250 ms toggle, inherited from Lab 3). Upon final confirmation in `TUNE_GREEN`, validate `draft_red == draft_green + draft_amber` before committing to `time_red`, `time_amber`, and `time_green`.
* **Rationale:** Prevents asymmetric intersection timing where one road turns Green while the orthogonal road is still Green or Amber. If invalid values are entered, `fsm_tuning` displays `"ERR: R!=G+A"` on the LCD and retains the previous valid durations.
* **Consequence:** `Huy` implements this validation in `fsm_tuning.c` and documents it in the report.
