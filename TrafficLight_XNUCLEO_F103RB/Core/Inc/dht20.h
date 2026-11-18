#ifndef INC_DHT20_H_
#define INC_DHT20_H_

#include "global.h"

/* DHT20 (AHT20 core) I2C address: 7-bit 0x38, HAL wants it shifted left by 1 */
#define DHT20_ADDR_7BIT             0x38U
#define DHT20_ADDR                  (DHT20_ADDR_7BIT << 1)

#define DHT20_STATUS_BUSY           0x80U   /* bit 7: measurement in progress */
#define DHT20_STATUS_CALIBRATED     0x18U   /* bits 3,4 must both be 1 after power-on */

#define DHT20_I2C_TIMEOUT_MS        5U      /* Max wait for one short I2C transfer */
#define DHT20_MEASURE_WAIT_TICKS    10U     /* 10 ticks = 100 ms (datasheet: >= 80 ms) */

/* Remember the I2C handle and check the sensor status byte.
 * Call once in main() after MX_I2C1_Init() and >= 100 ms after power-on.
 * Returns 1 if the sensor answered and is calibrated, 0 otherwise. */
uint8_t dht20_init(I2C_HandleTypeDef *hi2c);

/* Periodic task (PERIOD = 200 ticks = 2 s). Starts one measurement and
 * schedules a one-shot task to read the result 100 ms later.
 * Updates env_temperature and env_humidity in global.c. Never blocks. */
void dht20_read_task(void);

/* 1 while the last init/read succeeded, 0 if the sensor is missing or failing */
uint8_t dht20_is_ok(void);

#endif /* INC_DHT20_H_ */
