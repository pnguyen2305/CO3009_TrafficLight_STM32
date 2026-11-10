# Team Contributions and Review Log

> **Governance Rule:** Every work package must be committed by its author using their university Git identity, submitted via Pull Request to `dev`, and verified by the assigned peer reviewer before merge.

## 1. Member Ownership Summary (Phases 1–4: 09 Nov – 24 Nov 2026)

| Member | Student ID | GitHub Branch | Assigned Work Packages (Phases 1–4) | Agreed Contribution |
|---|---:|---|---|---:|
| **Nguyen Phuc Nguyen** | 2452859 | `Nguyen` | **Phase 1 (09–12 Nov):** Repo setup, `.ioc` (`Serial Wire`, clock check, `TIM2` 10ms), PA5 & Traffic Light 2-pin HW test<br>**Phase 2 (13–16 Nov):** Shared `global.h/.c`, `software_timer.h/.c`, `traffic_light.h/.c`<br>**Phase 3 (17–20 Nov):** `fsm_automatic.h/.c`, mode-switching integration<br>**Phase 4 (21–24 Nov):** Staggered `main.c` task table, `README.md`, Report Sections 1–2 | Equal (33.3%) |
| **Nguyen Viet Tung** | 2453391 | `Tung` | **Phase 1 (09–12 Nov):** `CHECKPOINTS.md`, `CONTRIBUTIONS.md`, 4-Button HW polarity test<br>**Phase 2 (13–16 Nov):** `button.h/.c` (10ms 3-register debounce + consume-on-read + 1s long-press)<br>**Phase 3 (17–20 Nov):** `lcd.h/.c` I2C driver, FSM diagrams, `fsm_manual.h/.c`<br>**Phase 4 (21–24 Nov):** Edge-case validation, Report Sections 3–4 | Equal (33.3%) |
| **Nguyen Van Minh Huy** | 2452401 | `Huy` | **Phase 1 (09–12 Nov):** `DECISIONS.md`, I2C1 bus scanner HW test (`DHT20` & `LCD` addresses)<br>**Phase 2 (13–16 Nov):** Mandatory O(1) `scheduler.h/.c` + one-shot & `scheduler_design.md`<br>**Phase 3 (17–20 Nov):** `dht20.h/.c` I2C sensor driver, `fsm_tuning.h/.c` (`Red = Green + Amber`)<br>**Phase 4 (21–24 Nov):** 30-min stability test log, Report Sections 5–6 & final PDF export | Equal (33.3%) |

## 2. Phase Pull Request and Commit Traceability Matrix

| Phase | Work Package ID | Author | Target Files | Planned Commit Prefix | Reviewer | PR # | Status |
|---|---|---|---|---|---|---|---|
| **P1 (09–12 Nov)** | `WP1.1-Plan-IOC` | Nguyen | `.gitignore`, `PROJECT_PLAN.md`, `README.md`, `*.ioc` | `docs:` / `hw:` | Tung | `PR #1` | Planned |
| **P1 (09–12 Nov)** | `WP1.2-LightTest` | Nguyen | `Core/Src/main.c`, `Docs/hardware_test_notes.md` | `test:` | Tung | `PR #2` | Planned |
| **P1 (09–12 Nov)** | `WP1.3-BtnTest` | Tung | `CHECKPOINTS.md`, `CONTRIBUTIONS.md`, `Core/Src/main.c` | `docs:` / `test:` | Huy | `PR #3` | Planned |
| **P1 (09–12 Nov)** | `WP1.4-I2CTest` | Huy | `DECISIONS.md`, `Core/Src/main.c`, `Docs/hardware_test_notes.md` | `docs:` / `test:` | Nguyen | `PR #4` | Planned |
| **P2 (13–16 Nov)** | `WP2.1-Scheduler` | Huy | `Core/Inc/scheduler.h`, `Core/Src/scheduler.c`, `Docs/scheduler_design.md` | `scheduler:` | Nguyen | `PR #5` | Planned |
| **P2 (13–16 Nov)** | `WP2.2-TimerLight` | Nguyen | `global.h/.c`, `software_timer.h/.c`, `traffic_light.h/.c` | `timer:` / `traffic:` | Huy | `PR #6` | Planned |
| **P2 (13–16 Nov)** | `WP2.3-ButtonDrv` | Tung | `Core/Inc/button.h`, `Core/Src/button.c` | `button:` | Huy | `PR #7` | Planned |
| **P3 (17–20 Nov)** | `WP3.1-LCD-Manual` | Tung | `lcd.h/.c`, `Docs/fsm_diagrams.drawio`, `fsm_manual.h/.c` | `lcd:` / `fsm_manual:` | Huy | `PR #8` | Planned |
| **P3 (17–20 Nov)** | `WP3.2-DHT-Tuning` | Huy | `dht20.h/.c`, `fsm_tuning.h/.c`, `DECISIONS.md` | `dht20:` / `fsm_tuning:` | Tung | `PR #9` | Planned |
| **P3 (17–20 Nov)** | `WP3.3-Auto-Sync` | Nguyen | `fsm_automatic.h/.c`, `global.c`, `Core/Src/main.c` | `fsm_automatic:` / `fsm:` | Tung | `PR #10` | Planned |
| **P4 (21–24 Nov)** | `WP4.1-SysFinal` | Nguyen | `Core/Src/main.c`, `README.md`, Report Sec 1–2 | `system:` / `docs:` | Tung | `PR #11` | Planned |
| **P4 (21–24 Nov)** | `WP4.2-EdgeCases` | Tung | `fsm_manual.c`, `CHECKPOINTS.md`, Report Sec 3–4 | `fix:` / `report:` | Huy | `PR #12` | Planned |
| **P4 (21–24 Nov)** | `WP4.3-QA-Report` | Huy | `Docs/test_plan_and_results.md`, Report Sec 5–6, Final PDF | `test:` / `report:` | Nguyen | `PR #13` | Planned |
