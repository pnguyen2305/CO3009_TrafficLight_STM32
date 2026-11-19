#include "fsm_manual.h"

/* Safe 2-second amber transition duration */
#define MANUAL_AMBER_DURATION_S     2U

/* Helper: switch to Tuning mode safely */
static void transition_to_tuning(void) {
    clearAllTimers();
    clearAllButtonFlags();
    lcd_needs_update = 1U;
    status = TUNE_INIT;
}

/* Helper: 1-second amber countdown */
static void update_manual_amber_countdown(int *countdown_ptr) {
    if (timer_flag[TIMER_1S_TICK] == 1U) {
        setTimer(TIMER_1S_TICK, 1000U);
        if (*countdown_ptr > 1) {
            (*countdown_ptr)--;
        }
        lcd_needs_update = 1U;
    }
}

void fsm_manual_run(void) {
    switch (status) {
        case MAN_INIT:
            /* Deterministic safe entry: Road 1 Red, Road 2 Green */
            clearAllTimers();
            clearAllButtonFlags();

            road1_countdown = 0;
            road2_countdown = 0;
            traffic_set(ROAD_1, LIGHT_RED);
            traffic_set(ROAD_2, LIGHT_GREEN);

            lcd_needs_update = 1U;
            status = MAN_RED1_GREEN2;
            break;

        case MAN_RED1_GREEN2:
            traffic_set(ROAD_1, LIGHT_RED);
            traffic_set(ROAD_2, LIGHT_GREEN);

            if (isButtonPressed(BTN_MODIFY)) {
                road1_countdown = MANUAL_AMBER_DURATION_S;
                road2_countdown = MANUAL_AMBER_DURATION_S;
                traffic_set(ROAD_1, LIGHT_RED);
                traffic_set(ROAD_2, LIGHT_AMBER);
                setTimer(TIMER_TRAFFIC, MANUAL_AMBER_DURATION_S * 1000U);
                setTimer(TIMER_1S_TICK, 1000U);
                lcd_needs_update = 1U;
                status = MAN_RED1_AMBER2;
            } else if (isButtonPressed(BTN_MODE)) {
                transition_to_tuning();
            }
            break;

        case MAN_RED1_AMBER2:
            traffic_set(ROAD_1, LIGHT_RED);
            traffic_set(ROAD_2, LIGHT_AMBER);
            update_manual_amber_countdown(&road2_countdown);

            /* Ignore BTN_MODIFY during Amber buffer */
            (void)isButtonPressed(BTN_MODIFY);

            if (timer_flag[TIMER_TRAFFIC] == 1U) {
                clearTimer(TIMER_TRAFFIC);
                clearTimer(TIMER_1S_TICK);
                road1_countdown = 0;
                road2_countdown = 0;
                traffic_set(ROAD_1, LIGHT_GREEN);
                traffic_set(ROAD_2, LIGHT_RED);
                lcd_needs_update = 1U;
                status = MAN_GREEN1_RED2;
            } else if (isButtonPressed(BTN_MODE)) {
                transition_to_tuning();
            }
            break;

        case MAN_GREEN1_RED2:
            traffic_set(ROAD_1, LIGHT_GREEN);
            traffic_set(ROAD_2, LIGHT_RED);

            if (isButtonPressed(BTN_MODIFY)) {
                road1_countdown = MANUAL_AMBER_DURATION_S;
                road2_countdown = MANUAL_AMBER_DURATION_S;
                traffic_set(ROAD_1, LIGHT_AMBER);
                traffic_set(ROAD_2, LIGHT_RED);
                setTimer(TIMER_TRAFFIC, MANUAL_AMBER_DURATION_S * 1000U);
                setTimer(TIMER_1S_TICK, 1000U);
                lcd_needs_update = 1U;
                status = MAN_AMBER1_RED2;
            } else if (isButtonPressed(BTN_MODE)) {
                transition_to_tuning();
            }
            break;

        case MAN_AMBER1_RED2:
            traffic_set(ROAD_1, LIGHT_AMBER);
            traffic_set(ROAD_2, LIGHT_RED);
            update_manual_amber_countdown(&road1_countdown);

            /* Ignore BTN_MODIFY during Amber buffer */
            (void)isButtonPressed(BTN_MODIFY);

            if (timer_flag[TIMER_TRAFFIC] == 1U) {
                clearTimer(TIMER_TRAFFIC);
                clearTimer(TIMER_1S_TICK);
                road1_countdown = 0;
                road2_countdown = 0;
                traffic_set(ROAD_1, LIGHT_RED);
                traffic_set(ROAD_2, LIGHT_GREEN);
                lcd_needs_update = 1U;
                status = MAN_RED1_GREEN2;
            } else if (isButtonPressed(BTN_MODE)) {
                transition_to_tuning();
            }
            break;

        default:
            break;
    }
}