#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "global.h"

#define BUTTON_IS_PRESSED GPIO_PIN_RESET
#define BUTTON_IS_RELEASED GPIO_PIN_SET

#define DURATION_FOR_LONG_PRESS 100U
#define DURATION_FOR_AUTO_REPEAT 25U

void button_init(void);
void button_reading(void);
uint8_t isButtonPressed(uint8_t index);
uint8_t isButtonLongPressed(uint8_t index);
void clearAllButtonFlags(void);

#endif /* INC_BUTTON_H_ */
