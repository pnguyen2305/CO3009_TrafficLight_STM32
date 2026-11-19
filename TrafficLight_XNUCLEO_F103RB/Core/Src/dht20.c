#include "dht20.h"
#include "scheduler.h"
#include <stddef.h>

/*
 * DHT20 temperature & humidity driver, non-blocking (split-phase) version
 * -----------------------------------------------------------------------
 * The sensor needs ~80 ms to finish a measurement. The usual example code
 * does   send command -> HAL_Delay(80) -> read   which freezes the whole
 * super loop for 80 ms: buttons and FSMs would stop responding.
 *
 * Here the work is split into two short scheduler tasks:
 *
 *   t = 0 ms    dht20_read_task()       send "measure" command (3 bytes)
 *                                       add one-shot task in 10 ticks
 *   t = 100 ms  dht20_fetch_result()    read 7 bytes, check, convert
 *
 * Between the two, the CPU keeps running the other tasks. Each I2C
 * transfer is only a few bytes (< 1 ms at 100 kHz), so neither task blocks.
 *
 * Frame read from the sensor (7 bytes):
 *   [0] status  [1..3] humidity 20 bit  [3..5] temperature 20 bit  [6] CRC-8
 *   byte 3 is shared: high nibble = end of humidity, low nibble = start of temp.
 */

static I2C_HandleTypeDef *dht20_hi2c = NULL;
static uint8_t dht20_ok = 0;        /* sensor answered and is calibrated */
static uint8_t dht20_pending = 0;   /* a measurement is in progress */

/* CRC-8 from the datasheet: polynomial x^8 + x^5 + x^4 + 1 (0x31), init 0xFF.
 * Computed over bytes 0..5; must equal byte 6. Detects noisy I2C reads. */
static uint8_t dht20_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0xFFU;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8U; bit++) {
            if (crc & 0x80U) {
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

uint8_t dht20_init(I2C_HandleTypeDef *hi2c) {
    uint8_t sensor_status = 0;

    dht20_hi2c = hi2c;
    dht20_ok = 0;
    dht20_pending = 0;

    if (dht20_hi2c == NULL) {
        return 0;
    }

    /* Datasheet "send 0x71 to read status": 0x71 is simply the read address
     * byte (0x38 << 1 | 1), which HAL_I2C_Master_Receive sends for us. */
    if (HAL_I2C_Master_Receive(dht20_hi2c, DHT20_ADDR, &sensor_status, 1,
                               DHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        return 0;   /* no ACK: sensor not plugged in or wrong address */
    }

    if ((sensor_status & DHT20_STATUS_CALIBRATED) != DHT20_STATUS_CALIBRATED) {
        /* Factory calibration not loaded. A fresh DHT20 should never report
         * this; we just mark the sensor as not OK and retry later. */
        return 0;
    }

    dht20_ok = 1;
    return 1;
}

/* One-shot task, runs 100 ms after the measure command was sent */
static void dht20_fetch_result(void) {
    uint8_t frame[7];
    uint32_t raw_humidity;
    uint32_t raw_temperature;

    dht20_pending = 0;

    if (HAL_I2C_Master_Receive(dht20_hi2c, DHT20_ADDR, frame, sizeof(frame),
                               DHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        dht20_ok = 0;           /* bus error: re-init on next period */
        return;
    }

    if (frame[0] & DHT20_STATUS_BUSY) {
        return;                 /* not finished yet: keep old values this round */
    }

    if (dht20_crc8(frame, 6) != frame[6]) {
        return;                 /* corrupted frame: keep old values */
    }

    /* Rebuild the two 20-bit raw values */
    raw_humidity    = ((uint32_t)frame[1] << 12)
                    | ((uint32_t)frame[2] << 4)
                    | ((uint32_t)frame[3] >> 4);
    raw_temperature = (((uint32_t)frame[3] & 0x0FU) << 16)
                    | ((uint32_t)frame[4] << 8)
                    |  (uint32_t)frame[5];

    /* Datasheet formulas, 2^20 = 1048576:
     *   RH [%] = raw / 2^20 * 100
     *   T  [C] = raw / 2^20 * 200 - 50 */
    env_humidity    = ((float)raw_humidity * 100.0f) / 1048576.0f;
    env_temperature = ((float)raw_temperature * 200.0f) / 1048576.0f - 50.0f;

    lcd_needs_update = 1;
}

void dht20_read_task(void) {
    uint8_t cmd[3] = {0xACU, 0x33U, 0x00U};    /* "trigger measurement" */

    if (dht20_hi2c == NULL) {
        return;                 /* dht20_init() was never called */
    }

    if (dht20_ok == 0) {
        /* Sensor missing or failed last time (e.g. unplugged and plugged
         * back in): try to re-init now, measure on the next period. */
        (void)dht20_init(dht20_hi2c);
        return;
    }

    if (dht20_pending) {
        return;                 /* previous measurement not read yet */
    }

    if (HAL_I2C_Master_Transmit(dht20_hi2c, DHT20_ADDR, cmd, sizeof(cmd),
                                DHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        dht20_ok = 0;
        return;
    }

    /* Instead of HAL_Delay(80): ask the scheduler to call us back later.
     * PERIOD = 0 means one-shot, the scheduler drops it after it runs. */
    dht20_pending = 1;
    if (SCH_Add_Task(dht20_fetch_result, DHT20_MEASURE_WAIT_TICKS, 0) == NO_TASK_ID) {
        dht20_pending = 0;      /* task table full: try again next period */
    }
}

uint8_t dht20_is_ok(void) {
    return dht20_ok;
}
