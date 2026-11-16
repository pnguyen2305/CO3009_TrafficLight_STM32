#include "traffic_light.h"

static void traffic_write_pins(GPIO_TypeDef *portA, uint16_t pinA,
                               GPIO_TypeDef *portB, uint16_t pinB,
                               TrafficColor_t color) {
    GPIO_PinState bitA = GPIO_PIN_RESET;
    GPIO_PinState bitB = GPIO_PIN_RESET;

    switch (color) {
        case LIGHT_OFF:
            bitA = GPIO_PIN_RESET;
            bitB = GPIO_PIN_RESET;
            break;
        case LIGHT_GREEN:
            bitA = GPIO_PIN_RESET;
            bitB = GPIO_PIN_SET;
            break;
        case LIGHT_AMBER:
            bitA = GPIO_PIN_SET;
            bitB = GPIO_PIN_RESET;
            break;
        case LIGHT_RED:
            bitA = GPIO_PIN_SET;
            bitB = GPIO_PIN_SET;
            break;
        default:
            bitA = GPIO_PIN_RESET;
            bitB = GPIO_PIN_RESET;
            break;
    }

    HAL_GPIO_WritePin(portA, pinA, bitA);
    HAL_GPIO_WritePin(portB, pinB, bitB);
}

void traffic_init(void) {
    traffic_set(ROAD_1, LIGHT_OFF);
    traffic_set(ROAD_2, LIGHT_OFF);
}

void traffic_set(uint8_t road, TrafficColor_t color) {
    if (road == ROAD_1) {
        traffic_write_pins(TL1_A_GPIO_Port, TL1_A_Pin,
                           TL1_B_GPIO_Port, TL1_B_Pin, color);
    } else if (road == ROAD_2) {
        traffic_write_pins(TL2_A_GPIO_Port, TL2_A_Pin,
                           TL2_B_GPIO_Port, TL2_B_Pin, color);
    }
}

void traffic_blink_color(TrafficColor_t color) {
    static uint8_t blink_on = 0;
    blink_on = !blink_on;

    if (blink_on) {
        traffic_set(ROAD_1, color);
        traffic_set(ROAD_2, color);
    } else {
        traffic_set(ROAD_1, LIGHT_OFF);
        traffic_set(ROAD_2, LIGHT_OFF);
    }
}
