#include "software_timer.h"

static uint32_t timer_counter[MAX_TIMERS] = {0};
uint8_t timer_flag[MAX_TIMERS] = {0};

void setTimer(uint8_t index, uint32_t duration_ms) {
    if (index >= MAX_TIMERS) return;

    if (duration_ms == 0) {
        timer_counter[index] = 0;
        timer_flag[index] = 1;
        return;
    }

    uint32_t ticks = duration_ms / TICK_MS;
    timer_counter[index] = (ticks > 0) ? ticks : 1;
    timer_flag[index] = 0;
}

void clearTimer(uint8_t index) {
    if (index >= MAX_TIMERS) return;
    timer_counter[index] = 0;
    timer_flag[index] = 0;
}

void clearAllTimers(void) {
    for (uint8_t i = 0; i < MAX_TIMERS; i++) {
        timer_counter[i] = 0;
        timer_flag[i] = 0;
    }
}

void timer_run(void) {
    for (uint8_t i = 0; i < MAX_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            if (timer_counter[i] == 0) {
                timer_flag[i] = 1;
            }
        }
    }
}
