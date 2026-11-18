#include "fsm_tuning.h"

/*
 * Timing Adjustment (Tuning) mode
 * --------------------------------
 *   TUNE_INIT -> TUNE_RED -> TUNE_AMBER -> TUNE_GREEN --SET--> check R == G + A
 *                                                        |-- valid   -> save, AUTO_INIT
 *                                                        |-- invalid -> TUNE_ERROR (1 s) -> AUTO_INIT
 *   BTN_MODE in any TUNE_* state: discard drafts and go to AUTO_INIT.
 *
 * Buttons in TUNE_RED / TUNE_AMBER / TUNE_GREEN:
 *   BTN_MODIFY : +1 s (hold > 1 s: +1 every 250 ms)
 *   BTN_AUX    : -1 s (hold > 1 s: -1 every 250 ms)
 *   BTN_SET    : confirm this value and go to the next state
 *
 * Rule (PROJECT_PLAN 3.2): no HAL_* call in this file. LEDs go through
 * traffic_light.c, timing through software_timer.c, buttons through button.c.
 */

/* ---------------------------------------------------------------- helpers */

/* Leave Tuning mode. Every mode switch goes through an *_INIT state with
 * clean timers and button flags, so no stale press leaks into Automatic. */
static void tune_exit_to_auto(void) {
    clearAllTimers();
    clearAllButtonFlags();
    lcd_needs_update = 1;
    status = AUTO_INIT;
}

/* Enter the next editing state: turn all lights off and restart the blink
 * timer, so the new color starts its blink from a clean "off" phase. */
static void tune_enter(SystemState_t next) {
    traffic_set(ROAD_1, LIGHT_OFF);
    traffic_set(ROAD_2, LIGHT_OFF);
    setTimer(TIMER_BLINK_2HZ, TUNE_BLINK_HALF_MS);
    lcd_needs_update = 1;
    status = next;
}

/* Blink the color being tuned on both roads at 2 Hz.
 * The timer expires every 250 ms; each expiry toggles the light once,
 * so one full on+off cycle takes 500 ms = 2 Hz. */
static void tune_blink(TrafficColor_t color) {
    if (timer_flag[TIMER_BLINK_2HZ] == 1) {
        setTimer(TIMER_BLINK_2HZ, TUNE_BLINK_HALF_MS);
        traffic_blink_color(color);
    }
}

/* Read MODIFY / AUX and change *value inside [TUNE_MIN_SECONDS, max_value].
 *
 * A short press sets button_flag, a held button sets button_long_flag every
 * 250 ms. Both flags are consume-on-read, so we must read BOTH every tick.
 * We use bitwise '|' (not '||') on purpose: '||' would skip the second call
 * when the first is true, leaving that flag unread until the next tick. */
static void tune_adjust(int *value, int max_value) {
    uint8_t inc = isButtonPressed(BTN_MODIFY) | isButtonLongPressed(BTN_MODIFY);
    uint8_t dec = isButtonPressed(BTN_AUX)    | isButtonLongPressed(BTN_AUX);

    if (inc && (*value < max_value)) {
        (*value)++;
        lcd_needs_update = 1;
    }
    if (dec && (*value > TUNE_MIN_SECONDS)) {
        (*value)--;
        lcd_needs_update = 1;
    }
}

/* Final check when SET is pressed in TUNE_GREEN.
 * While road 1 is Red, road 2 goes Green then Amber. If Red is longer than
 * Green + Amber, both roads are Red for a while; if shorter, road 1 would
 * turn Green while road 2 is still Green/Amber (unsafe). So we need
 * Red == Green + Amber exactly. */
static uint8_t tune_is_valid(void) {
    return (draft_red == draft_green + draft_amber) ? 1U : 0U;
}

/* ------------------------------------------------------------------- FSM */

void fsm_tuning_run(void) {
    switch (status) {
        case TUNE_INIT:
            /* Fresh start: copy the active durations into the drafts.
             * Users edit drafts only; time_* changes only after validation. */
            clearAllTimers();
            clearAllButtonFlags();
            draft_red   = time_red;
            draft_amber = time_amber;
            draft_green = time_green;
            tune_enter(TUNE_RED);
            break;

        case TUNE_RED:
            if (isButtonPressed(BTN_MODE)) {     /* abort, keep old values */
                tune_exit_to_auto();
                break;
            }
            if (isButtonPressed(BTN_SET)) {
                tune_enter(TUNE_AMBER);
                break;
            }
            tune_adjust(&draft_red, TUNE_MAX_RED_SECONDS);
            tune_blink(LIGHT_RED);
            break;

        case TUNE_AMBER:
            if (isButtonPressed(BTN_MODE)) {
                tune_exit_to_auto();
                break;
            }
            if (isButtonPressed(BTN_SET)) {
                tune_enter(TUNE_GREEN);
                break;
            }
            tune_adjust(&draft_amber, TUNE_MAX_AMBER_SECONDS);
            tune_blink(LIGHT_AMBER);
            break;

        case TUNE_GREEN:
            if (isButtonPressed(BTN_MODE)) {
                tune_exit_to_auto();
                break;
            }
            if (isButtonPressed(BTN_SET)) {
                if (tune_is_valid()) {
                    /* Commit: from now on Automatic mode uses the new values */
                    time_red   = draft_red;
                    time_amber = draft_amber;
                    time_green = draft_green;
                    tune_exit_to_auto();
                } else {
                    /* Reject: time_* stay unchanged. Lights off, LCD shows
                     * the error (lcd_refresh_task reads status == TUNE_ERROR). */
                    traffic_set(ROAD_1, LIGHT_OFF);
                    traffic_set(ROAD_2, LIGHT_OFF);
                    clearTimer(TIMER_BLINK_2HZ);
                    setTimer(TIMER_TRAFFIC, TUNE_ERROR_SHOW_MS);
                    lcd_needs_update = 1;
                    status = TUNE_ERROR;
                }
                break;
            }
            tune_adjust(&draft_green, TUNE_MAX_GREEN_SECONDS);
            tune_blink(LIGHT_GREEN);
            break;

        case TUNE_ERROR:
            /* Buttons are ignored here; tune_exit_to_auto() clears any
             * press made during this second. */
            if (timer_flag[TIMER_TRAFFIC] == 1) {
                tune_exit_to_auto();
            }
            break;

        default:
            /* Not in Tuning mode: another FSM owns the system right now */
            break;
    }
}
