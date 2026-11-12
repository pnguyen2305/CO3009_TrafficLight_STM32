# CO3009 Course Project Work Plan & Technical Specification — Two-Road Traffic Light System

> **Planning baseline — 10 Nov 2026:** This document defines the 4-Phase implementation roadmap (09 Nov – 24 Nov 2026), Lab 1–4 technical upgrade rules, unified shared API & variable contracts (`global.h`), file ownership matrix, commit schedule, and verification workflow for the CO3009 Course Project (BTL) on the **XNUCLEO-F103RB** board (`STM32F103RB`).

## 1. Project Scope, Timeline, and Milestone Structure

### 1.1 Realistic 16-Day Schedule (Start: 09 Nov 2026 -> Finish: 24 Nov 2026, 10 Days Early)

Because the project starts on **09 Nov 2026** and the team targets completion on **24 Nov 2026** (10 days ahead of the official **04 Dec 2026** LMS deadline), the 16-day active window spans **2.5 calendar weeks** and is divided into **4 Technical Phases (`Phase 1` – `Phase 4`)** following the bottom-up architecture in Part B0 of the guide (`Hardware -> Scheduler & Core HAL -> I2C & FSMs -> Integration & Report`), rather than artificially calling 4-day blocks "weeks".

| Phase | Milestone Tag | Calendar Window | Real Calendar Week | Core Deliverable Scope |
|---|---|---|---|---|
| **Phase 1** | `m1-hardware` | **09 Nov – 12 Nov 2026** (4 days) | Week 1 (First half) | Steps 1–3: Repo setup, `.ioc` config (`Serial Wire`, clock tree, `TIM2` 10 ms), independent hardware tests (PA5, 2-pin Lights, 4 Buttons, I2C scan) |
| **Phase 2** | `m2-scheduler-hal` | **13 Nov – 16 Nov 2026** (4 days) | Week 1 (Second half) – Week 2 | Step 4 & Step 5A: Mandatory O(1) relative-delay `scheduler` (with one-shot support) + Core GPIO/Timer modules (`global`, `software_timer`, `button`, `traffic_light`) |
| **Phase 3** | `m3-fsm` | **17 Nov – 20 Nov 2026** (4 days) | Week 2 | Step 5B & Step 6: I2C drivers (`dht20`, `lcd`), formal FSM diagrams, and 3 state machines (`fsm_automatic`, `fsm_manual`, `fsm_tuning`) |
| **Phase 4** | `v1.0-final` | **21 Nov – 24 Nov 2026** (4 days) | Week 3 (Closeout) | Step 7: Full system integration, edge-case & 30-min stability testing, final PDF report, and `v1.0-final` submission tag (10 days before 04 Dec) |

### 1.2 Assessment Criteria and Technical Mapping (Part A5)

| No. | Criterion | Weight | Key Technical Enforcement in This Plan |
|---:|---|---:|---|
| 1 | **O(1) Scheduler** (`SCH_Add_Task`, `SCH_Delete_Task`, `SCH_Update` O(1), `SCH_Dispatch_Tasks`, one-shot) | **10%** | Relative-delay sorted array (`SCH_tasks_G`); `SCH_Update()` only decrements `SCH_tasks_G[0].Delay`; handles multiple zero-delay tasks per tick and `Period = 0` auto-deletion. |
| 2 | **State Machine Design & Safe Transitions** | **30%** | Unified `SystemState_t status` enum with explicit `AUTO_INIT`, `MAN_INIT`, `TUNE_INIT` entry states that reset timers, clear stale button flags, enforce a 2 s Amber buffer before road switching, and guarantee both roads are never Green simultaneously. |
| 3 | **Traffic Light Functions** (Automatic, Manual, Timing Adjustment) | **20%** | 4-phase Automatic mode with 1 s countdown; Manual override with Amber interlock; Tuning mode with 2 Hz LED blinking, live LCD preview, and `Red = Green + Amber` validation. |
| 4 | **Supporting Modules** (`software_timer`, `button`, `traffic_light`, `dht20`, `lcd`) | **10%** | Edge-triggered consume-on-read `isButtonPressed(i)` + 1 s long-press auto-repeat; 2-pin truth-table `traffic_set()`; non-blocking 2 s `dht20` I2C task; event-driven/throttled `lcd` I2C updates. |
| 5 | **Code Organization & Git History** | **10%** | Zero `HAL_*` calls in `fsm_*.c`; zero `HAL_Delay()` calls anywhere after Phase 1; strict `snake_case` naming; balanced individual commits & PR reviews across all 4 phases. |
| 6 | **Report & Live Demo** | **20%** | Complete PDF report (`CO3009_2452859_2453391_2452401_Report.pdf`) with schematic pin table, 2-pin light truth table, FSM diagrams, O(1) complexity proof, and `v1.0-final` commit hash. |

## 2. Key Differences and Upgrade Rules from Labs 1–4 to Course Project (BTL)

Because this project synthesizes and upgrades **Lab 1 (LED Animations)**, **Lab 2 (Timer Interrupt & Software Timer)**, **Lab 3 (Button Debouncing & Traffic Light FSM)**, and **Lab 4 (Cooperative Scheduler)** onto real **XNUCLEO-F103RB** hardware, all team members must follow these 6 critical adaptation rules:

| Topic | Previous Labs 1–4 (`STM32F103C6` on Proteus) | Course Project BTL (`XNUCLEO-F103RB` Real Board) | Mandatory Team Rule |
|---|---|---|---|
| **1. MCU & SWD Debug** | Selected `STM32F103C6`; `SYS -> Debug` left as `No Debug` (Proteus ignores SWD). | Uses `STM32F103RB` via **Board Selector -> NUCLEO-F103RB**. Leaving Debug off locks SWD after 1st flash. | **Nguyen** must set **SYS -> Debug = Serial Wire** in `.ioc` before generating code; **Tung** verifies in PR #1. |
| **2. Timer Clock (`f_TIM`)** | Bare `STM32F103C6` defaults to `8 MHz` HSI (`PSC=7999, ARR=9` -> `10 ms`). | `NUCLEO-F103RB` Board Selector defaults to `64 MHz` PLL (`APB1 Timer = 64 MHz`) unless reconfigured. | Check **Clock Configuration** tab in `.ioc`: if `f_TIM = 64 MHz`, set `PSC = 6399, ARR = 99` (or set APB1 Timer Clock to `8 MHz` with `PSC = 7999, ARR = 9`) so tick is exactly `10 ms`. |
| **3. Traffic Light Pins** | Lab 1 & 3 used 6 separate GPIO pins (3 active-low LEDs per road). | Grove Traffic Light module uses **2 pins per road** (`D3–D4` and `D5–D6`) encoding 4 states (`00, 01, 10, 11`). | **Nguyen** determines the exact 2-pin truth table in Phase 1 (Step 3.2) and encapsulates it inside `traffic_set(road, color)`. |
| **4. Display Output** | Lab 1–3 used 7-segment LEDs (`display7SEG`, `update7SEG`) scanned at 50 Hz. | **No 7-segment LEDs.** All countdowns, modes, tuning values, and DHT20 data are shown on the **I2C 16x2 LCD**. | Remove all 7-segment code; **Tung** implements `lcd.h/.c` using the 8-bit shifted I2C address (`lcd_addr_7bit << 1`). |
| **5. Button Reading API** | Lab 3 `is_button_pressed(i)` returned the raw debounced **level** (`1` while held), requiring a separate FSM per button. | BTL Step 6 (`if (isButtonPressed(0)) status = MAN_INIT;`) and Section B8 require **consume-on-read event flags**. | **Tung** implements 3-sample debounce (`KeyReg0..KeyReg3`) where `isButtonPressed(i)` returns `1` once per press and **clears `button_flag[i] = 0` immediately**. |
| **6. Scheduler Complexity & ISR** | Lab 2/3 called `timer_run()` & `button_reading()` in ISR; Lab 4 Sec 2.3.4 showed an O(n) loop in `SCH_Update()`. | `HAL_TIM_PeriodElapsedCallback` calls **only `SCH_Update()`**. `SCH_Update()` **must be O(1)**. `timer_run` & `button_reading` run as 10 ms scheduled tasks. | **Huy** implements a sorted relative-delay task queue where `SCH_Update()` only decrements `SCH_tasks_G[0].Delay`. |

## 3. Unified Shared API and Variable Contract (Plug-and-Play Integration Specification)

To guarantee that code written independently by **Nguyen**, **Tung**, and **Huy** compiles and integrates on the first try without naming or unit mismatches, all modules must strictly adhere to the interfaces below.

### 3.1 Shared Header Contract: `Core/Inc/global.h` (Author: Nguyen)

```c
#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "main.h"
#include <stdint.h>

/* 1. System Tick & Constants */
#define TICK_MS                 10U     // TIM2 interrupt period in ms
#define NUM_BUTTONS             4U      // 4 Grove buttons on D7, D8, D9, D10
#define MAX_TIMERS              6U      // Software timer channels

/* 2. Logical Button Indices (D7..D10) */
#define BTN_MODE                0U      // D7 (PA8): Cycle Mode (Auto -> Manual -> Tuning -> Auto)
#define BTN_MODIFY              1U      // D8 (PA9): Manual switch road / Tuning increment (+1)
#define BTN_SET                 2U      // D9 (PC7): Tuning confirm & advance (Red -> Amber -> Green)
#define BTN_AUX                 3U      // D10 (PB6): Tuning decrement (-1) or reset default

/* 3. Software Timer Channel Allocation */
#define TIMER_TRAFFIC           0U      // Main phase timer for Automatic / Manual Amber transition
#define TIMER_1S_TICK           1U      // 1000 ms timer for 1-second countdown decrement
#define TIMER_BLINK_2HZ         2U      // 250 ms timer for 2 Hz LED blinking in Tuning Mode
#define TIMER_LCD_REFRESH       3U      // Periodic LCD screen refresh timer

/* 4. Road & Light State Definitions */
#define ROAD_1                  1U
#define ROAD_2                  2U

typedef enum {
    LIGHT_OFF = 0,
    LIGHT_RED,
    LIGHT_AMBER,
    LIGHT_GREEN
} TrafficColor_t;

/* 5. Unified System State Machine Enum */
typedef enum {
    INIT_SYSTEM = 0,

    /* Automatic Mode States (fsm_automatic.c - Nguyen) */
    AUTO_INIT,
    AUTO_RED1_GREEN2,
    AUTO_RED1_AMBER2,
    AUTO_GREEN1_RED2,
    AUTO_AMBER1_RED2,

    /* Manual Mode States (fsm_manual.c - Tung) */
    MAN_INIT,
    MAN_RED1_GREEN2,
    MAN_RED1_AMBER2,    // 2s safety Amber before switching to GREEN1_RED2
    MAN_GREEN1_RED2,
    MAN_AMBER1_RED2,    // 2s safety Amber before switching to RED1_GREEN2

    /* Timing Adjustment Mode States (fsm_tuning.c - Huy) */
    TUNE_INIT,
    TUNE_RED,           // Adjust Red duration (1..99s), blink Red LEDs at 2Hz
    TUNE_AMBER,         // Adjust Amber duration (1..98s), blink Amber LEDs at 2Hz
    TUNE_GREEN          // Adjust Green duration (1..98s), blink Green LEDs at 2Hz
} SystemState_t;

/* 6. Shared Global Variables (Defined in global.c) */
extern SystemState_t status;

extern int time_red;            // Validated active Red duration (seconds, default: 5)
extern int time_amber;          // Validated active Amber duration (seconds, default: 2)
extern int time_green;          // Validated active Green duration (seconds, default: 3)

extern int draft_red;           // Temporary Red duration edited in Tuning mode
extern int draft_amber;         // Temporary Amber duration edited in Tuning mode
extern int draft_green;         // Temporary Green duration edited in Tuning mode

extern int road1_countdown;     // Remaining seconds for Road 1 current phase
extern int road2_countdown;     // Remaining seconds for Road 2 current phase

extern float env_temperature;   // Latest DHT20 temperature (C)
extern float env_humidity;      // Latest DHT20 humidity (%)
extern uint8_t lcd_needs_update;// Dirty flag to trigger immediate LCD redraw on state/value change

void global_init(void);

#endif /* INC_GLOBAL_H_ */
```

### 3.2 Supporting Module Function Prototypes & Unit Conventions (Steps 4–6)

| Module Header | Owner | Exact Public Function Prototypes | Parameter Unit & Integration Contract |
|---|---|---|---|
| `scheduler.h` | **Huy** | `void SCH_Init(void);`<br>`uint32_t SCH_Add_Task(void (*pTask)(void), uint32_t DELAY, uint32_t PERIOD);`<br>`uint8_t SCH_Delete_Task(uint32_t taskID);`<br>`void SCH_Update(void);`<br>`void SCH_Dispatch_Tasks(void);` | **`DELAY` and `PERIOD` are in 10 ms ticks** (e.g., `50` = 500 ms, `100` = 1 s, `200` = 2 s). `PERIOD = 0` denotes a one-shot task. `SCH_Update()` is called exclusively inside `HAL_TIM_PeriodElapsedCallback`. |
| `software_timer.h` | **Nguyen** | `extern uint8_t timer_flag[MAX_TIMERS];`<br>`void setTimer(uint8_t index, uint32_t duration_ms);`<br>`void clearTimer(uint8_t index);`<br>`void clearAllTimers(void);`<br>`void timer_run(void);` | **`duration_ms` is in milliseconds** (e.g., `setTimer(TIMER_TRAFFIC, time_green * 1000)` or `setTimer(TIMER_1S_TICK, 1000)`). Internally divides by `TICK_MS (10)`. `timer_run` is added to scheduler with `PERIOD = 1` (10 ms). |
| `button.h` | **Tung** | `void button_init(void);`<br>`void button_reading(void);`<br>`uint8_t isButtonPressed(uint8_t index);`<br>`uint8_t isButtonLongPressed(uint8_t index);`<br>`void clearAllButtonFlags(void);` | `button_reading` runs as a scheduler task with `PERIOD = 1` (10 ms). `isButtonPressed(i)` and `isButtonLongPressed(i)` **auto-clear their flag to `0` upon returning `1`**. Call `clearAllButtonFlags()` whenever switching modes. |
| `traffic_light.h` | **Nguyen** | `void traffic_init(void);`<br>`void traffic_set(uint8_t road, TrafficColor_t color);`<br>`void traffic_blink_color(TrafficColor_t color);` | `road` is `ROAD_1 (1)` or `ROAD_2 (2)`. `traffic_set()` writes the 2 GPIO pins (`TL1_A/B` or `TL2_A/B`). `traffic_blink_color()` toggles the selected color on both roads at 2 Hz for Tuning mode. |
| `dht20.h` | **Huy** | `uint8_t dht20_init(I2C_HandleTypeDef *hi2c);`<br>`void dht20_read_task(void);` | Uses 8-bit shifted address `0x38 << 1`. `dht20_read_task` is scheduled with `PERIOD = 200` (every 2 seconds) and updates `env_temperature` and `env_humidity`. |
| `lcd.h` | **Tung** | `void lcd_init(I2C_HandleTypeDef *hi2c);`<br>`void lcd_clear(void);`<br>`void lcd_goto_xy(uint8_t row, uint8_t col);`<br>`void lcd_send_string(const char *str);`<br>`void lcd_send_num(int num);`<br>`void lcd_refresh_task(void);` | Uses shifted address (`lcd_addr << 1`). `lcd_refresh_task` runs every `20` ticks (200 ms) or when `lcd_needs_update == 1`, formatting Line 0 (Mode + Road states/countdowns) and Line 1 (Tuning value or `T:xx.xC H:xx.x%`). |
| `fsm_automatic.h`<br>`fsm_manual.h`<br>`fsm_tuning.h` | **Nguyen**<br>**Tung**<br>**Huy** | `void fsm_automatic_run(void);`<br>`void fsm_manual_run(void);`<br>`void fsm_tuning_run(void);` | Each runs as a scheduler task (`PERIOD = 1` tick = 10 ms). Zero `HAL_*` calls allowed inside `fsm_*.c`. Mode transitions always enter via `AUTO_INIT`, `MAN_INIT`, or `TUNE_INIT`. |

### 3.3 Mode Transition & Safety Rules (30% Assessment Focus)

1. **`Automatic -> Manual` (Press `BTN_MODE` in any `AUTO_*` state):**
   - `fsm_automatic_run()` executes `clearAllTimers(); clearAllButtonFlags(); status = MAN_INIT;`.
   - `MAN_INIT` inspects which road was previously active (or defaults safely): if Road 2 was Green/Amber, it enters `MAN_RED1_GREEN2` (Road 1 Red, Road 2 Green); if Road 1 was Green/Amber, it enters `MAN_GREEN1_RED2` (Road 1 Green, Road 2 Red). Both roads are **never** Green simultaneously.
   - When the user presses `BTN_MODIFY` in `MAN_RED1_GREEN2` to switch right-of-way, the FSM **does not jump directly to `MAN_GREEN1_RED2`**. Instead, it transitions to `MAN_RED1_AMBER2`, sets `setTimer(TIMER_TRAFFIC, time_amber * 1000)`, and automatically advances to `MAN_GREEN1_RED2` when the Amber timer expires.
2. **`Manual -> Tuning` (Press `BTN_MODE` in any `MAN_*` state):**
   - `fsm_manual_run()` executes `clearAllTimers(); clearAllButtonFlags(); status = TUNE_INIT;`.
   - `TUNE_INIT` copies `draft_red = time_red; draft_amber = time_amber; draft_green = time_green;`, turns off normal lights, starts `setTimer(TIMER_BLINK_2HZ, 250)`, and transitions to `TUNE_RED`.
3. **Duration Constraint Validation in `Tuning` Mode (`TUNE_RED -> TUNE_AMBER -> TUNE_GREEN -> AUTO_INIT`):**
   - In `TUNE_RED`, `TUNE_AMBER`, `TUNE_GREEN`: `BTN_MODIFY` increments the draft value (`1..99`, with long-press auto-increment every 250 ms); `BTN_AUX` decrements (`1..99`); `BTN_SET` advances `TUNE_RED -> TUNE_AMBER -> TUNE_GREEN`.
   - When pressing `BTN_SET` in `TUNE_GREEN` (final confirmation): the FSM checks the invariant **`draft_red == draft_green + draft_amber`**.
     - If valid (`draft_red == draft_green + draft_amber`): commit `time_red = draft_red; time_amber = draft_amber; time_green = draft_green;` and transition to `AUTO_INIT`.
     - If invalid (`draft_red != draft_green + draft_amber`): reject the invalid set (or auto-reconcile `time_red = draft_green + draft_amber` if within `2..99`, otherwise keep previous valid durations), display `"ERR: R!=G+A"` on the LCD for 1 second, and transition cleanly to `AUTO_INIT`.
   - Pressing `BTN_MODE` at any time during Tuning mode aborts unsaved draft changes and transitions directly to `AUTO_INIT`.

## 4. Complete Repository Directory Structure and File Ownership Matrix

```text
CO3009_TrafficLight_STM32/
├── .gitignore
├── README.md
├── PROJECT_PLAN.md
├── CHECKPOINTS.md
├── CONTRIBUTIONS.md
├── DECISIONS.md
├── TrafficLight_XNUCLEO_F103RB.ioc
├── Docs/
│   ├── hardware_test_notes.md
│   ├── scheduler_design.md
│   ├── fsm_diagrams.drawio
│   ├── fsm_specification.md
│   └── test_plan_and_results.md
├── Report/
│   ├── Template_Report_BTL_CO3009.docx
│   └── CO3009_2452859_2453391_2452401_Report.pdf
└── Core/
    ├── Inc/
    │   ├── main.h
    │   ├── global.h
    │   ├── scheduler.h
    │   ├── software_timer.h
    │   ├── button.h
    │   ├── traffic_light.h
    │   ├── dht20.h
    │   ├── lcd.h
    │   ├── fsm_automatic.h
    │   ├── fsm_manual.h
    │   └── fsm_tuning.h
    └── Src/
        ├── main.c
        ├── stm32f1xx_it.c
        ├── global.c
        ├── scheduler.c
        ├── software_timer.c
        ├── button.c
        ├── traffic_light.c
        ├── dht20.c
        ├── lcd.c
        ├── fsm_automatic.c
        ├── fsm_manual.c
        └── fsm_tuning.c
```

| File Path | Layer / Category | Owner | Reviewer | Target Phase |
|---|---|---|---|---|
| `.gitignore`, `PROJECT_PLAN.md`, `README.md` | Governance & Setup | **Nguyen** | Tung | Phase 1 (09–12 Nov) |
| `CHECKPOINTS.md`, `CONTRIBUTIONS.md` | Governance | **Tung** | Huy | Phase 1–4 |
| `DECISIONS.md` | Governance | **Huy** | Nguyen | Phase 1–4 |
| `TrafficLight_XNUCLEO_F103RB.ioc` | Hardware Config | **Nguyen** | Tung | Phase 1 (09–12 Nov) |
| `Docs/hardware_test_notes.md` | Hardware Verification | **All (Nguyen, Tung, Huy)** | All | Phase 1 (09–12 Nov) |
| `Core/Inc/scheduler.h`, `Core/Src/scheduler.c`, `Docs/scheduler_design.md` | Mandatory O(1) Scheduler | **Huy** | Nguyen | Phase 2 (13–16 Nov) |
| `Core/Inc/global.h`, `Core/Src/global.c` | Shared State Contract | **Nguyen** | Tung | Phase 2 (13–16 Nov) |
| `Core/Inc/software_timer.h`, `Core/Src/software_timer.c` | Supporting Module | **Nguyen** | Huy | Phase 2 (13–16 Nov) |
| `Core/Inc/traffic_light.h`, `Core/Src/traffic_light.c` | Supporting Module | **Nguyen** | Tung | Phase 2 (13–16 Nov) |
| `Core/Inc/button.h`, `Core/Src/button.c` | Supporting Module | **Tung** | Huy | Phase 2 (13–16 Nov) |
| `Core/Inc/lcd.h`, `Core/Src/lcd.c` | I2C Supporting Module | **Tung** | Nguyen | Phase 3 (17–20 Nov) |
| `Core/Inc/dht20.h`, `Core/Src/dht20.c` | I2C Supporting Module | **Huy** | Tung | Phase 3 (17–20 Nov) |
| `Docs/fsm_diagrams.drawio`, `Docs/fsm_specification.md` | FSM Design Diagrams | **Tung** (with Nguyen & Huy) | All | Phase 3 (17–20 Nov) |
| `Core/Inc/fsm_automatic.h`, `Core/Src/fsm_automatic.c` | Application FSM | **Nguyen** | Tung | Phase 3 (17–20 Nov) |
| `Core/Inc/fsm_manual.h`, `Core/Src/fsm_manual.c` | Application FSM | **Tung** | Huy | Phase 3 (17–20 Nov) |
| `Core/Inc/fsm_tuning.h`, `Core/Src/fsm_tuning.c` | Application FSM | **Huy** | Nguyen | Phase 3 (17–20 Nov) |
| `Core/Src/main.c` | System Entry & Task Table | **Nguyen** (Lead) | Tung & Huy | Phase 1 & Phase 4 |
| `Docs/test_plan_and_results.md` | QA & 30-Min Stability Log | **Huy** | Tung | Phase 4 (21–24 Nov) |
| `Report/CO3009_2452859_2453391_2452401_Report.pdf` | Final Report Deliverable | **All (2 sections each)** | All | Phase 4 (21–24 Nov) |

## 5. Hardware Pin Mapping Table (Derived from XNUCLEO-F103RB Schematic Figure 2)

| Device | Connection Header | STM32F103RB Pin | GPIO / Peripheral Mode | User Label in `.ioc` | Owner |
|---|---|---|---|---|---|
| **Traffic Light Module 1 (Road 1)** | `D3` | `PB3` | GPIO Output Push-Pull | `TL1_A` | Nguyen |
| **Traffic Light Module 1 (Road 1)** | `D4` | `PB5` | GPIO Output Push-Pull | `TL1_B` | Nguyen |
| **Traffic Light Module 2 (Road 2)** | `D5` | `PB4` | GPIO Output Push-Pull | `TL2_A` | Nguyen |
| **Traffic Light Module 2 (Road 2)** | `D6` | `PB10` | GPIO Output Push-Pull | `TL2_B` | Nguyen |
| **Button Module 1 (`BTN_MODE`)** | `D7` | `PA8` | GPIO Input (Pull-Up) | `BTN_1` | Tung |
| **Button Module 1 (`BTN_MODIFY`)** | `D8` | `PA9` | GPIO Input (Pull-Up) | `BTN_2` | Tung |
| **Button Module 2 (`BTN_SET`)** | `D9` | `PC7` | GPIO Input (Pull-Up) | `BTN_3` | Tung |
| **Button Module 2 (`BTN_AUX`)** | `D10` | `PB6` | GPIO Input (Pull-Up) | `BTN_4` | Tung |
| **DHT20 Sensor & 16x2 LCD** | Red I2C Grove Slot | `PB8` (`SCL`), `PB9` (`SDA`) | `I2C1` (100 kHz Standard) | `I2C1_SCL`, `I2C1_SDA` | Huy |
| **On-board LEDs / USER Button** | Built-in | `PA5`, `PC9` / `PC13` | Output Push-Pull / Input | `LED_PA5`, `LED_PC9`, `USER_BTN` | Nguyen |

## 6. Detailed 4-Phase Task Assignment and Commit Schedule (09 Nov – 24 Nov 2026)

### Phase 1 (09 Nov – 12 Nov 2026) — Repo Governance, `.ioc` Config & Hardware Testing (Steps 1–3)
- **Milestone Tag:** `m1-hardware` (Cutoff: 23:59, 12 Nov 2026)

| Member | Assigned Tasks in Phase 1 | Target Files | Planned Git Commits |
|---|---|---|---|
| **Nguyen** | Initialize GitHub repo, `.gitignore`, `PROJECT_PLAN.md`, `README.md`; create STM32CubeIDE project (`NUCLEO-F103RB`, `SYS -> Debug = Serial Wire`, check `f_TIM` clock tree, configure `TIM2` 10 ms and `D3–D10` + `I2C1`); run Step 3.1 (PA5 blink) & Step 3.2 (record 2-pin traffic light truth table). | `.gitignore`, `PROJECT_PLAN.md`, `README.md`, `*.ioc`, `Core/Src/main.c`, `Docs/hardware_test_notes.md` | 1. `docs: add project plan, gitignore and initial README (step 1)`<br>2. `hw: configure NUCLEO-F103RB ioc with Serial Wire, I2C1 and 10ms TIM2 (step 2)`<br>3. `test: verify PA5 blink and record 2-pin traffic light truth table (step 3.1-3.2)` |
| **Tung** | Create `CHECKPOINTS.md` and `CONTRIBUTIONS.md`; run Step 3.3 button hardware test for `D7–D10` (`PA8, PA9, PC7, PB6`) and `PC13`; record active pressed logic level (`GPIO_PIN_RESET` vs `GPIO_PIN_SET`) in `Docs/hardware_test_notes.md`. | `CHECKPOINTS.md`, `CONTRIBUTIONS.md`, `Core/Src/main.c`, `Docs/hardware_test_notes.md` | 1. `docs: initialize phase checkpoints and contributions tracker (step 1)`<br>2. `test: verify button active logic levels on D7-D10 and PC13 (step 3.3)` |
| **Huy** | Create `DECISIONS.md`; implement Step 3.4 I2C bus scanner (`addr = 1..127`, shifting `addr << 1`) in `main.c`; record verified 7-bit and 8-bit addresses for DHT20 (`0x38`) and LCD in `Docs/hardware_test_notes.md`. | `DECISIONS.md`, `Core/Src/main.c`, `Docs/hardware_test_notes.md` | 1. `docs: add architectural decisions log and lab-to-btl upgrade rules (step 1)`<br>2. `test: run I2C1 address scan and record DHT20 and LCD addresses (step 3.4)` |

### Phase 2 (13 Nov – 16 Nov 2026) — Mandatory O(1) Scheduler & Core GPIO/Timer Modules (Step 4 & Step 5A)
- **Milestone Tag:** `m2-scheduler-hal` (Cutoff: 23:59, 16 Nov 2026)

| Member | Assigned Tasks in Phase 2 | Target Files | Planned Git Commits |
|---|---|---|---|
| **Huy** | Implement O(1) relative-delay sorted array scheduler (`SCH_Init`, `SCH_Add_Task`, `SCH_Delete_Task` with delay restoration, O(1) `SCH_Update`, `SCH_Dispatch_Tasks` handling multiple `Delay == 0` tasks and `Period = 0` one-shot tasks); verify with 500 ms + 1000 ms + 3 s one-shot demo; write `Docs/scheduler_design.md`. | `Core/Inc/scheduler.h`, `Core/Src/scheduler.c`, `Docs/scheduler_design.md`, `Core/Src/main.c` | 1. `scheduler: implement O(1) relative-delay sorted queue and dispatcher (step 4.1)`<br>2. `scheduler: add one-shot task support and multi-period verification demo (step 4.2)` |
| **Nguyen** | Implement `global.h/.c` (exact shared contract from Section 3.1), `software_timer.h/.c` (`setTimer`, `clearTimer`, `clearAllTimers`, `timer_run`), and `traffic_light.h/.c` (`traffic_set`, `traffic_blink_color` using Phase 1 truth table). | `Core/Inc/global.h`, `Core/Src/global.c`, `Core/Inc/software_timer.h`, `Core/Src/software_timer.c`, `Core/Inc/traffic_light.h`, `Core/Src/traffic_light.c` | 1. `timer: implement shared global contract and 10ms software timer module (step 5)`<br>2. `traffic: implement 2-pin traffic light driver and 2Hz blink helper (step 5)` |
| **Tung** | Implement `button.h/.c` with 10 ms sampling, 3-register debounce filter (`KeyReg0..3`), consume-on-read `isButtonPressed(i)`, 1 s long-press auto-repeat (`isButtonLongPressed(i)`), and `clearAllButtonFlags()`; test button-triggered light state changes on the scheduler. | `Core/Inc/button.h`, `Core/Src/button.c`, `CHECKPOINTS.md`, `CONTRIBUTIONS.md` | 1. `button: implement 10ms 3-register debouncer and consume-on-read press flags (step 5)`<br>2. `button: add 1s long-press auto-repeat and clearAllButtonFlags helper (step 5)` |

### Phase 3 (17 Nov – 20 Nov 2026) — I2C Drivers (`DHT20`, `LCD`) & State Machine Implementation (Step 5B & Step 6)
- **Milestone Tag:** `m3-fsm` (Cutoff: 23:59, 20 Nov 2026)

| Member | Assigned Tasks in Phase 3 | Target Files | Planned Git Commits |
|---|---|---|---|
| **Tung** | Implement `lcd.h/.c` I2C driver and non-blocking `lcd_refresh_task()`; lead FSM diagram design (`Docs/fsm_diagrams.drawio`, `Docs/fsm_specification.md`); implement `fsm_manual.h/.c` (`MAN_INIT`, `MAN_RED1_GREEN2`, `MAN_RED1_AMBER2`, `MAN_GREEN1_RED2`, `MAN_AMBER1_RED2`). | `Core/Inc/lcd.h`, `Core/Src/lcd.c`, `Docs/fsm_diagrams.drawio`, `Docs/fsm_specification.md`, `Core/Inc/fsm_manual.h`, `Core/Src/fsm_manual.c` | 1. `lcd: implement I2C 16x2 LCD driver and non-blocking refresh task (step 5)`<br>2. `docs: add state machine diagrams and mode transition specification (step 6.1)`<br>3. `fsm_manual: implement manual road switching with 2s amber safety buffer (step 6.2)` |
| **Huy** | Implement `dht20.h/.c` I2C temperature & humidity driver (`dht20_read_task` running every 200 ticks = 2 s); implement `fsm_tuning.h/.c` (`TUNE_INIT`, `TUNE_RED`, `TUNE_AMBER`, `TUNE_GREEN`, 2 Hz LED blinking, `draft_red == draft_green + draft_amber` constraint check). | `Core/Inc/dht20.h`, `Core/Src/dht20.c`, `Core/Inc/fsm_tuning.h`, `Core/Src/fsm_tuning.c`, `DECISIONS.md` | 1. `dht20: implement periodic 2s I2C temperature and humidity driver (step 5)`<br>2. `fsm_tuning: implement duration tuning FSM with 2Hz blink and R=G+A check (step 6.2)` |
| **Nguyen** | Implement `fsm_automatic.h/.c` (`AUTO_INIT` and 4 traffic phases with 1-second countdown timer `TIMER_1S_TICK`); wire mode-transition handoffs (`AUTO_INIT -> MAN_INIT -> TUNE_INIT -> AUTO_INIT`) and register tasks in `main.c`. | `Core/Inc/fsm_automatic.h`, `Core/Src/fsm_automatic.c`, `Core/Src/global.c`, `Core/Src/main.c` | 1. `fsm_automatic: implement 4-phase cycle and 1s road countdown logic (step 6.2)`<br>2. `fsm: integrate mode-switching entry states and scheduler task table (step 6.2)` |

### Phase 4 (21 Nov – 24 Nov 2026) — System Integration, Edge-Case QA, Report Writing & Early Release (Step 7)
- **Milestone Tag:** `v1.0-final` (Cutoff: 23:59, 24 Nov 2026 — 10 days before official 04 Dec deadline)

| Member | Assigned Tasks in Phase 4 | Target Files | Planned Git Commits |
|---|---|---|---|
| **Nguyen** | Audit entire `Core/Src/fsm_*.c` to confirm zero `HAL_*` calls and zero `HAL_Delay()` calls; stagger initial task delays in `main.c` (`0, 1, 2, 5, 10` ticks as recommended in Lab 4 Sec 2.3.11 to prevent task jitter); write Report Sections 1 & 2 and finalize `README.md`. | `Core/Src/main.c`, `README.md`, `Report/Template_Report_BTL_CO3009.docx` | 1. `system: stagger initial task delays in main and audit HAL layer separation (step 7.1)`<br>2. `docs: finalize README build guide and author report hardware sections (step 7.2-7.3)` |
| **Tung** | Run edge-case test suite (rapid button mashing, mode switch during Amber phase, invalid tuning durations `Red != Green + Amber`, Green-Green mutual exclusion check); write Report Sections 3 & 4 (Supporting Modules & State Machine Design); finalize `CHECKPOINTS.md` & `CONTRIBUTIONS.md`. | `Core/Src/fsm_manual.c`, `CHECKPOINTS.md`, `CONTRIBUTIONS.md`, `Report/Template_Report_BTL_CO3009.docx` | 1. `fix: verify amber transition edge cases and LCD mode-switch redraws (step 6-7)`<br>2. `report: author supporting modules and state machine sections in report (step 7.2)` |
| **Huy** | Execute 30-minute continuous hardware stability test and document results in `Docs/test_plan_and_results.md`; write Report Sections 5 & 6 (O(1) Scheduler Justification, Experimental Results & Conclusion); export final PDF `CO3009_2452859_2453391_2452401_Report.pdf` with final commit hash. | `Docs/test_plan_and_results.md`, `DECISIONS.md`, `Report/CO3009_2452859_2453391_2452401_Report.pdf` | 1. `test: log 30-minute continuous stability run and edge-case test matrix (step 7.1)`<br>2. `report: author O(1) scheduler analysis and export final submission PDF (step 7.2)` |

## 7. Production Scheduler Task Table in `main.c` (Anti-Jitter Staggered Offsets from Lab 4 Sec 2.3.11)

Following Lab 4 Section 2.3.11 ("Dealing with task overlap"), tasks in `main.c` are registered with staggered initial delays (`DELAY` in 10 ms ticks) so heavy I2C transactions never collide on the same tick:

```c
/* Inside main(), before while(1): */
SCH_Init();
global_init();

SCH_Add_Task(timer_run,          0,   1);   // Every 10 ms (1 tick), offset 0
SCH_Add_Task(button_reading,     0,   1);   // Every 10 ms (1 tick), offset 0
SCH_Add_Task(fsm_automatic_run,  1,   1);   // Every 10 ms (1 tick), offset 1 tick
SCH_Add_Task(fsm_manual_run,     1,   1);   // Every 10 ms (1 tick), offset 1 tick
SCH_Add_Task(fsm_tuning_run,     1,   1);   // Every 10 ms (1 tick), offset 1 tick
SCH_Add_Task(lcd_refresh_task,   3,  20);   // Every 200 ms (20 ticks), offset 3 ticks
SCH_Add_Task(dht20_read_task,    8, 200);   // Every 2000 ms (200 ticks), offset 8 ticks
SCH_Add_Task(status_led_blink,   5,  50);   // Every 500 ms (50 ticks) heartbeat on PA5
```