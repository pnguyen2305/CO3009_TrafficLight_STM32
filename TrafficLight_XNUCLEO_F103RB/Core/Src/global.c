#include "global.h"

SystemState_t status = INIT_SYSTEM;

int time_red   = 5;
int time_amber = 2;
int time_green = 3;

int draft_red   = 5;
int draft_amber = 2;
int draft_green = 3;

int road1_countdown = 0;
int road2_countdown = 0;

float env_temperature = 0.0f;
float env_humidity    = 0.0f;
uint8_t lcd_needs_update = 1;

void global_init(void) {
    status = AUTO_INIT;

    time_red   = 5;
    time_amber = 2;
    time_green = 3;

    draft_red   = time_red;
    draft_amber = time_amber;
    draft_green = time_green;

    road1_countdown = time_red;
    road2_countdown = time_green;

    env_temperature = 0.0f;
    env_humidity    = 0.0f;
    lcd_needs_update = 1;
}
