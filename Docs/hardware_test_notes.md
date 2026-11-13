# Hardware Verification Notes (Phase 1 — Steps 3.1 to 3.4)

> **Board:** XNUCLEO-F103RB (`STM32F103RB`) | **Milestone:** `m1-hardware`

## 1. Clock & Timer Configuration (Step 2.4 — Nguyen)
- **Debug Interface:** `SYS -> Debug = Serial Wire` (enabled in `.ioc` to prevent SWD lockout).
- **Timer Tick (`TIM2`):** Configured for `10 ms` periodic interrupt (`100 Hz` scheduler tick).

## 2. On-Board LED & Traffic Light 2-Pin Truth Table (Steps 3.1 – 3.2 — Nguyen)
- **Step 3.1 (`PA5` Blink):** Verified via `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)` with `500 ms` delay.
- **Step 3.2 (2-Pin Traffic Light Modules on `D3–D4` / `PB3–PB5` and `D5–D6` / `PB4–PB10`):**

| `Signal A` (`TLx_A`) | `Signal B` (`TLx_B`) | Observed Light State |
|---|---|---|
| `0` | `0` | `Pending board verification` |
| `0` | `1` | `Pending board verification` |
| `1` | `0` | `Pending board verification` |
| `1` | `1` | `Pending board verification` |

## 3. Push Button Logic Levels (Step 3.3 — Tung)

| Button Label | Arduino Header | STM32 Pin | GPIO Mode | Logic Level (Released) | Logic Level (Pressed) |
|---|---|---|---|---|---|
| `BTN_1` (`BTN_MODE`) | `D7` | `PA8` | Input Pull-Up | `1` (`GPIO_PIN_SET`) | `0` (`GPIO_PIN_RESET` - verify on board) |
| `BTN_2` (`BTN_MODIFY`) | `D8` | `PA9` | Input Pull-Up | `1` (`GPIO_PIN_SET`) | `0` (`GPIO_PIN_RESET` - verify on board) |
| `BTN_3` (`BTN_SET`) | `D9` | `PC7` | Input Pull-Up | `1` (`GPIO_PIN_SET`) | `0` (`GPIO_PIN_RESET` - verify on board) |
| `BTN_4` (`BTN_AUX`) | `D10` | `PB6` | Input Pull-Up | `1` (`GPIO_PIN_SET`) | `0` (`GPIO_PIN_RESET` - verify on board) |
| `USER_BTN` | Built-in | `PC13` | Input | `1` (`GPIO_PIN_SET`) | `0` (`GPIO_PIN_RESET` - verify on board) |

## 4. I2C1 Bus Address Scan (Step 3.4 — Huy)
- **Peripheral Pins:** `I2C1_SCL = PB8`, `I2C1_SDA = PB9` (Red Grove I2C slots, `100 kHz` Standard Mode).
- **Scanner Implementation:** Implemented in `TrafficLight_XNUCLEO_F103RB/Core/Src/main.c` (`USER CODE BEGIN 2`), scanning 7-bit addresses `addr = 1..127` using `HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 1, 10)` and storing results in `found[8]` (7-bit) and `found_8bit[8]` (8-bit left-shifted for STM32 HAL).

| Device on `I2C1` | 7-bit Address (`found[i]`) | 8-bit Shifted Address (`found_8bit[i] = addr << 1`) | Target Driver Module | Verification Status |
|---|---|---|---|---|
| **DHT20 Temperature & Humidity Sensor** | `0x38` (`56` dec) | `0x70` (`112` dec) | `Core/Src/dht20.c` (Huy) | Fixed per DHT20 datasheet; confirm `HAL_OK` during board scan |
| **Grove 16x2 I2C LCD** | `0x27` / `0x3E` / `0x3F` *(confirm on board)* | `0x4E` / `0x7C` / `0x7E` *(confirm on board)* | `Core/Src/lcd.c` (Tung) | Inspect `found[]` via STM32CubeIDE breakpoint on physical board |