#include "button.h"

static GPIO_TypeDef* const BUTTON_PORT[NUM_BUTTONS] = {
    BTN_1_GPIO_Port,
    BTN_2_GPIO_Port,
    BTN_3_GPIO_Port,
    BTN_4_GPIO_Port
};

static const uint16_t BUTTON_PIN[NUM_BUTTONS] = {
    BTN_1_Pin,
    BTN_2_Pin,
    BTN_3_Pin,
    BTN_4_Pin
};

static GPIO_PinState KeyReg0[NUM_BUTTONS];
static GPIO_PinState KeyReg1[NUM_BUTTONS];
static GPIO_PinState KeyReg2[NUM_BUTTONS];
static GPIO_PinState KeyReg3[NUM_BUTTONS];

static uint16_t TimeOutForKeyPress[NUM_BUTTONS];

static uint8_t button_flag[NUM_BUTTONS];
static uint8_t button_long_flag[NUM_BUTTONS];

void button_init(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        KeyReg0[i] = BUTTON_IS_RELEASED;
        KeyReg1[i] = BUTTON_IS_RELEASED;
        KeyReg2[i] = BUTTON_IS_RELEASED;
        KeyReg3[i] = BUTTON_IS_RELEASED;
        TimeOutForKeyPress[i] = DURATION_FOR_LONG_PRESS;
        button_flag[i] = 0;
        button_long_flag[i] = 0;
    }
}

void button_reading(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        KeyReg2[i] = KeyReg1[i];
        KeyReg1[i] = KeyReg0[i];
        KeyReg0[i] = HAL_GPIO_ReadPin(BUTTON_PORT[i], BUTTON_PIN[i]);

        if ((KeyReg1[i] == KeyReg0[i]) && (KeyReg1[i] == KeyReg2[i])) {
            if (KeyReg3[i] != KeyReg2[i]) {
                KeyReg3[i] = KeyReg2[i];

                if (KeyReg2[i] == BUTTON_IS_PRESSED) {
                    button_flag[i] = 1;
                    TimeOutForKeyPress[i] = DURATION_FOR_LONG_PRESS;
                } else {
                    button_long_flag[i] = 0;
                    TimeOutForKeyPress[i] = DURATION_FOR_LONG_PRESS;
                }
            } else {
                if (KeyReg2[i] == BUTTON_IS_PRESSED) {
                    if (TimeOutForKeyPress[i] > 0) {
                        TimeOutForKeyPress[i]--;
                        if (TimeOutForKeyPress[i] == 0) {
                            button_long_flag[i] = 1;
                            TimeOutForKeyPress[i] = DURATION_FOR_AUTO_REPEAT;
                        }
                    }
                }
            }
        }
    }
}

uint8_t isButtonPressed(uint8_t index) {
    if (index >= NUM_BUTTONS) return 0;
    if (button_flag[index] == 1) {
        button_flag[index] = 0;
        return 1;
    }
    return 0;
}

uint8_t isButtonLongPressed(uint8_t index) {
    if (index >= NUM_BUTTONS) return 0;
    if (button_long_flag[index] == 1) {
        button_long_flag[index] = 0;
        return 1;
    }
    return 0;
}

void clearAllButtonFlags(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        button_flag[i] = 0;
        button_long_flag[i] = 0;
    }
}
