#include "lcd.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    LCD_TYPE_NONE = 0,
    LCD_TYPE_PCF8574,       /* HD44780 4-bit via PCF8574 (0x27 / 0x3F) */
    LCD_TYPE_GROVE_JHD1802  /* Native I2C Grove 16x2 LCD (0x3E) */
} LcdType_t;

static I2C_HandleTypeDef *lcd_i2c = NULL;
static uint16_t lcd_addr = LCD_ADDR_PCF8574_1;
static LcdType_t lcd_type = LCD_TYPE_NONE;

/* PCF8574 control bit masks */
#define PCF_BL      0x08U
#define PCF_EN      0x04U
#define PCF_RS      0x01U

/* Low-level I2C command transmit */
static void lcd_send_cmd(uint8_t cmd) {
    if (lcd_i2c == NULL || lcd_type == LCD_TYPE_NONE) return;

    if (lcd_type == LCD_TYPE_GROVE_JHD1802) {
        uint8_t buf[2] = {0x80U, cmd};
        HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, buf, 2U, 10U);
    } else {
        uint8_t upper = cmd & 0xF0U;
        uint8_t lower = (uint8_t)((cmd << 4) & 0xF0U);
        uint8_t data[4];
        data[0] = upper | PCF_BL | PCF_EN;
        data[1] = upper | PCF_BL;
        data[2] = lower | PCF_BL | PCF_EN;
        data[3] = lower | PCF_BL;
        HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, data, 4U, 10U);
    }
}

/* Low-level I2C character transmit */
static void lcd_send_data(uint8_t ch) {
    if (lcd_i2c == NULL || lcd_type == LCD_TYPE_NONE) return;

    if (lcd_type == LCD_TYPE_GROVE_JHD1802) {
        uint8_t buf[2] = {0x40U, ch};
        HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, buf, 2U, 10U);
    } else {
        uint8_t upper = ch & 0xF0U;
        uint8_t lower = (uint8_t)((ch << 4) & 0xF0U);
        uint8_t data[4];
        data[0] = upper | PCF_BL | PCF_RS | PCF_EN;
        data[1] = upper | PCF_BL | PCF_RS;
        data[2] = lower | PCF_BL | PCF_RS | PCF_EN;
        data[3] = lower | PCF_BL | PCF_RS;
        HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, data, 4U, 10U);
    }
}

/* Auto-detect I2C LCD type/address and initialize 16x2 display */
void lcd_init(I2C_HandleTypeDef *hi2c) {
    lcd_i2c = hi2c;
    if (lcd_i2c == NULL) return;

    if (HAL_I2C_IsDeviceReady(lcd_i2c, LCD_ADDR_GROVE_NATIVE, 2U, 10U) == HAL_OK) {
        lcd_addr = LCD_ADDR_GROVE_NATIVE;
        lcd_type = LCD_TYPE_GROVE_JHD1802;
    } else if (HAL_I2C_IsDeviceReady(lcd_i2c, LCD_ADDR_PCF8574_1, 2U, 10U) == HAL_OK) {
        lcd_addr = LCD_ADDR_PCF8574_1;
        lcd_type = LCD_TYPE_PCF8574;
    } else if (HAL_I2C_IsDeviceReady(lcd_i2c, LCD_ADDR_PCF8574_2, 2U, 10U) == HAL_OK) {
        lcd_addr = LCD_ADDR_PCF8574_2;
        lcd_type = LCD_TYPE_PCF8574;
    } else {
        /* Fallback address */
        lcd_addr = LCD_ADDR_PCF8574_1;
        lcd_type = LCD_TYPE_PCF8574;
    }

    HAL_Delay(50);

    if (lcd_type == LCD_TYPE_GROVE_JHD1802) {
        lcd_send_cmd(0x38U); /* 2-line 5x8 */
        HAL_Delay(5);
        lcd_send_cmd(0x0CU); /* Display ON */
        HAL_Delay(2);
        lcd_send_cmd(0x01U); /* Clear */
        HAL_Delay(2);
        lcd_send_cmd(0x06U); /* Auto-increment */
    } else {
        /* HD44780 4-bit init */
        lcd_send_cmd(0x30U);
        HAL_Delay(5);
        lcd_send_cmd(0x30U);
        HAL_Delay(1);
        lcd_send_cmd(0x30U);
        HAL_Delay(1);
        lcd_send_cmd(0x20U); /* 4-bit mode */
        HAL_Delay(1);
        lcd_send_cmd(0x28U); /* 2-line 5x8 */
        lcd_send_cmd(0x08U); /* Display OFF */
        lcd_send_cmd(0x01U); /* Clear */
        HAL_Delay(2);
        lcd_send_cmd(0x06U); /* Auto-increment */
        lcd_send_cmd(0x0CU); /* Display ON */
    }
}

void lcd_clear(void) {
    lcd_send_cmd(0x01U);
}

void lcd_goto_xy(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0U) ? (0x80U + col) : (0xC0U + col);
    lcd_send_cmd(addr);
}

void lcd_send_string(const char *str) {
    if (str == NULL) return;
    while (*str != '\0') {
        lcd_send_data((uint8_t)(*str));
        str++;
    }
}

void lcd_send_num(int num) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    lcd_send_string(buf);
}

/* Pad text to 16 chars to avoid flicker */
static void lcd_write_line(uint8_t row, const char *text) {
    char line[17];
    size_t len = strlen(text);
    for (size_t i = 0; i < 16U; i++) {
        line[i] = (i < len) ? text[i] : ' ';
    }
    line[16] = '\0';
    lcd_goto_xy(row, 0U);
    lcd_send_string(line);
}

/* Periodic 200ms scheduler task */
void lcd_refresh_task(void) {
    char line0[17];
    char line1[17];

    int t_int = (int)env_temperature;
    int t_dec = (int)((env_temperature - (float)t_int) * 10.0f);
    if (t_dec < 0) t_dec = -t_dec;

    int h_int = (int)env_humidity;
    int h_dec = (int)((env_humidity - (float)h_int) * 10.0f);
    if (h_dec < 0) h_dec = -h_dec;

    switch (status) {
        case AUTO_INIT:
        case AUTO_RED1_GREEN2:
            snprintf(line0, sizeof(line0), "AUTO R1:%02d G2:%02d", road1_countdown, road2_countdown);
            snprintf(line1, sizeof(line1), "T:%2d.%1dC  H:%2d.%1d%%", t_int, t_dec, h_int, h_dec);
            break;

        case AUTO_RED1_AMBER2:
            snprintf(line0, sizeof(line0), "AUTO R1:%02d A2:%02d", road1_countdown, road2_countdown);
            snprintf(line1, sizeof(line1), "T:%2d.%1dC  H:%2d.%1d%%", t_int, t_dec, h_int, h_dec);
            break;

        case AUTO_GREEN1_RED2:
            snprintf(line0, sizeof(line0), "AUTO G1:%02d R2:%02d", road1_countdown, road2_countdown);
            snprintf(line1, sizeof(line1), "T:%2d.%1dC  H:%2d.%1d%%", t_int, t_dec, h_int, h_dec);
            break;

        case AUTO_AMBER1_RED2:
            snprintf(line0, sizeof(line0), "AUTO A1:%02d R2:%02d", road1_countdown, road2_countdown);
            snprintf(line1, sizeof(line1), "T:%2d.%1dC  H:%2d.%1d%%", t_int, t_dec, h_int, h_dec);
            break;

        case MAN_INIT:
        case MAN_RED1_GREEN2:
            snprintf(line0, sizeof(line0), "MAN  R1:RED 2:GR");
            snprintf(line1, sizeof(line1), "T:%2d.%1dC  H:%2d.%1d%%", t_int, t_dec, h_int, h_dec);
            break;

        case MAN_RED1_AMBER2:
            snprintf(line0, sizeof(line0), "MAN  R1:RED 2:AM");
            snprintf(line1, sizeof(line1), "Switching... %02ds", road2_countdown);
            break;

        case MAN_GREEN1_RED2:
            snprintf(line0, sizeof(line0), "MAN  R1:GR 2:RED");
            snprintf(line1, sizeof(line1), "T:%2d.%1dC  H:%2d.%1d%%", t_int, t_dec, h_int, h_dec);
            break;

        case MAN_AMBER1_RED2:
            snprintf(line0, sizeof(line0), "MAN  R1:AM 2:RED");
            snprintf(line1, sizeof(line1), "Switching... %02ds", road1_countdown);
            break;

        case TUNE_INIT:
        case TUNE_RED:
            snprintf(line0, sizeof(line0), "TUNE RED  :%02ds  ", draft_red);
            snprintf(line1, sizeof(line1), "R:%02d A:%02d G:%02d  ", draft_red, draft_amber, draft_green);
            break;

        case TUNE_AMBER:
            snprintf(line0, sizeof(line0), "TUNE AMBER:%02ds  ", draft_amber);
            snprintf(line1, sizeof(line1), "R:%02d A:%02d G:%02d  ", draft_red, draft_amber, draft_green);
            break;

        case TUNE_GREEN:
            snprintf(line0, sizeof(line0), "TUNE GREEN:%02ds  ", draft_green);
            snprintf(line1, sizeof(line1), "R:%02d A:%02d G:%02d  ", draft_red, draft_amber, draft_green);
            break;

        default:
            snprintf(line0, sizeof(line0), "TRAFFIC SYSTEM  ");
            snprintf(line1, sizeof(line1), "Initializing... ");
            break;
    }

    lcd_write_line(0U, line0);
    lcd_write_line(1U, line1);
    lcd_needs_update = 0U;
}