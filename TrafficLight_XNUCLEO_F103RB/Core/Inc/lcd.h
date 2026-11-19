#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "global.h"

/* Auto-detected 7-bit I2C addresses (shifted left by 1 for HAL) */
#define LCD_ADDR_PCF8574_1      (0x27U << 1)
#define LCD_ADDR_PCF8574_2      (0x3FU << 1)
#define LCD_ADDR_GROVE_NATIVE   (0x3EU << 1)

/* Public LCD driver API */
void lcd_init(I2C_HandleTypeDef *hi2c);
void lcd_clear(void);
void lcd_goto_xy(uint8_t row, uint8_t col);
void lcd_send_string(const char *str);
void lcd_send_num(int num);
void lcd_refresh_task(void);

#endif /* INC_LCD_H_ */
