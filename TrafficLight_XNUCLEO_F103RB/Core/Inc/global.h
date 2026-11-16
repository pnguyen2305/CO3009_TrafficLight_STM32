#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "main.h"
#include <stdint.h>

/* 1. System Tick & Constants */
#define TICK_MS                 10U     /* TIM2 interrupt period in ms */
#define NUM_BUTTONS             4U      /* 4 Grove buttons on D7, D8, D9, D10 */
#define MAX_TIMERS              6U      /* Software timer channels */

/* 2. Logical Button Indices (D7..D10) */
#define BTN_MODE                0U      /* D7 (PA8): Cycle Mode (Auto -> Manual -> Tuning -> Auto) */
#define BTN_MODIFY              1U      /* D8 (PA9): Manual switch road / Tuning increment (+1) */
#define BTN_SET                 2U      /* D9 (PC7): Tuning confirm & advance (Red -> Amber -> Green) */
#define BTN_AUX                 3U      /* D10 (PB6): Tuning decrement (-1) or reset default */

/* 3. Software Timer Channel Allocation */
#define TIMER_TRAFFIC           0U      /* Main phase timer for Automatic / Manual Amber transition */
#define TIMER_1S_TICK           1U      /* 1000 ms timer for 1-second countdown decrement */
#define TIMER_BLINK_2HZ         2U      /* 250 ms timer for 2 Hz LED blinking in Tuning Mode */
#define TIMER_LCD_REFRESH       3U      /* Periodic LCD screen refresh timer */

/* 4. Road & Light State Definitions */
#define ROAD_1                  1U
#define ROAD_2                  2U

typedef enum {
    LIGHT_OFF = 0,
    LIGHT_RED,
    LIGHT_AMBER,
    LIGHT_GREEN
} TrafficColor_t;

/* 5. Unified System State Machine Enum */
typedef enum {
    INIT_SYSTEM = 0,

    /* Automatic Mode States (fsm_automatic.c - Nguyen) */
    AUTO_INIT,
    AUTO_RED1_GREEN2,
    AUTO_RED1_AMBER2,
    AUTO_GREEN1_RED2,
    AUTO_AMBER1_RED2,

    /* Manual Mode States (fsm_manual.c - Tung) */
    MAN_INIT,
    MAN_RED1_GREEN2,
    MAN_RED1_AMBER2,    /* 2s safety Amber before switching to GREEN1_RED2 */
    MAN_GREEN1_RED2,
    MAN_AMBER1_RED2,    /* 2s safety Amber before switching to RED1_GREEN2 */

    /* Timing Adjustment Mode States (fsm_tuning.c - Huy) */
    TUNE_INIT,
    TUNE_RED,           /* Adjust Red duration (1..99s), blink Red LEDs at 2Hz */
    TUNE_AMBER,         /* Adjust Amber duration (1..98s), blink Amber LEDs at 2Hz */
    TUNE_GREEN          /* Adjust Green duration (1..98s), blink Green LEDs at 2Hz */
} SystemState_t;

/* 6. Shared Global Variables (Defined in global.c) */
extern SystemState_t status;

extern int time_red;            /* Validated active Red duration (seconds, default: 5) */
extern int time_amber;          /* Validated active Amber duration (seconds, default: 2) */
extern int time_green;          /* Validated active Green duration (seconds, default: 3) */

extern int draft_red;           /* Temporary Red duration edited in Tuning mode */
extern int draft_amber;         /* Temporary Amber duration edited in Tuning mode */
extern int draft_green;         /* Temporary Green duration edited in Tuning mode */

extern int road1_countdown;     /* Remaining seconds for Road 1 current phase */
extern int road2_countdown;     /* Remaining seconds for Road 2 current phase */

extern float env_temperature;   /* Latest DHT20 temperature (C) */
extern float env_humidity;      /* Latest DHT20 humidity (%) */
extern uint8_t lcd_needs_update;/* Dirty flag to trigger immediate LCD redraw */

void global_init(void);

#endif /* INC_GLOBAL_H_ */
