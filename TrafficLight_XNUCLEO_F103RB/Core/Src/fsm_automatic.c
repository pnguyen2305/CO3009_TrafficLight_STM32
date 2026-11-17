#include "fsm_automatic.h"

/* Helper: switch to Manual mode safely */
static void transition_to_manual(void) {
    clearAllTimers();
    clearAllButtonFlags();
    lcd_needs_update = 1;
    status = MAN_INIT;
}

/* Helper: 1-second countdown update */
static void update_countdown_1s(void) {
    if (timer_flag[TIMER_1S_TICK] == 1) {
        setTimer(TIMER_1S_TICK, 1000U);
        if (road1_countdown > 1) road1_countdown--;
        if (road2_countdown > 1) road2_countdown--;
        lcd_needs_update = 1;
    }
}

void fsm_automatic_run(void) {
    switch (status) {
        case AUTO_INIT:
            clearAllTimers();
            clearAllButtonFlags();

            road1_countdown = time_red;
            road2_countdown = time_green;

            traffic_set(ROAD_1, LIGHT_RED);
            traffic_set(ROAD_2, LIGHT_GREEN);

            setTimer(TIMER_TRAFFIC, (uint32_t)time_green * 1000U);
            setTimer(TIMER_1S_TICK, 1000U);

            lcd_needs_update = 1;
            status = AUTO_RED1_GREEN2;
            break;

        case AUTO_RED1_GREEN2:
            traffic_set(ROAD_1, LIGHT_RED);
            traffic_set(ROAD_2, LIGHT_GREEN);
            update_countdown_1s();

            if (timer_flag[TIMER_TRAFFIC] == 1) {
                road1_countdown = time_amber;
                road2_countdown = time_amber;
                traffic_set(ROAD_1, LIGHT_RED);
                traffic_set(ROAD_2, LIGHT_AMBER);
                setTimer(TIMER_TRAFFIC, (uint32_t)time_amber * 1000U);
                setTimer(TIMER_1S_TICK, 1000U);
                lcd_needs_update = 1;
                status = AUTO_RED1_AMBER2;
            }

            if (isButtonPressed(BTN_MODE)) {
                transition_to_manual();
            }
            break;

        case AUTO_RED1_AMBER2:
            traffic_set(ROAD_1, LIGHT_RED);
            traffic_set(ROAD_2, LIGHT_AMBER);
            update_countdown_1s();

            if (timer_flag[TIMER_TRAFFIC] == 1) {
                road1_countdown = time_green;
                road2_countdown = time_red;
                traffic_set(ROAD_1, LIGHT_GREEN);
                traffic_set(ROAD_2, LIGHT_RED);
                setTimer(TIMER_TRAFFIC, (uint32_t)time_green * 1000U);
                setTimer(TIMER_1S_TICK, 1000U);
                lcd_needs_update = 1;
                status = AUTO_GREEN1_RED2;
            }

            if (isButtonPressed(BTN_MODE)) {
                transition_to_manual();
            }
            break;

        case AUTO_GREEN1_RED2:
            traffic_set(ROAD_1, LIGHT_GREEN);
            traffic_set(ROAD_2, LIGHT_RED);
            update_countdown_1s();

            if (timer_flag[TIMER_TRAFFIC] == 1) {
                road1_countdown = time_amber;
                road2_countdown = time_amber;
                traffic_set(ROAD_1, LIGHT_AMBER);
                traffic_set(ROAD_2, LIGHT_RED);
                setTimer(TIMER_TRAFFIC, (uint32_t)time_amber * 1000U);
                setTimer(TIMER_1S_TICK, 1000U);
                lcd_needs_update = 1;
                status = AUTO_AMBER1_RED2;
            }

            if (isButtonPressed(BTN_MODE)) {
                transition_to_manual();
            }
            break;

        case AUTO_AMBER1_RED2:
            traffic_set(ROAD_1, LIGHT_AMBER);
            traffic_set(ROAD_2, LIGHT_RED);
            update_countdown_1s();

            if (timer_flag[TIMER_TRAFFIC] == 1) {
                road1_countdown = time_red;
                road2_countdown = time_green;
                traffic_set(ROAD_1, LIGHT_RED);
                traffic_set(ROAD_2, LIGHT_GREEN);
                setTimer(TIMER_TRAFFIC, (uint32_t)time_green * 1000U);
                setTimer(TIMER_1S_TICK, 1000U);
                lcd_needs_update = 1;
                status = AUTO_RED1_GREEN2;
            }

            if (isButtonPressed(BTN_MODE)) {
                transition_to_manual();
            }
            break;

        default:
            break;
    }
}
