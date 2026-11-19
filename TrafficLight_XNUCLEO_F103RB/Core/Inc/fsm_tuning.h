#ifndef INC_FSM_TUNING_H_
#define INC_FSM_TUNING_H_

#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "button.h"

/* Duration limits (seconds) for the values edited in Tuning mode.
 * Red is the longest phase (Red = Green + Amber), so Amber and Green can be
 * at most 98 s: with the other one at 1 s, Red is still <= 99 s. */
#define TUNE_MIN_SECONDS        1
#define TUNE_MAX_RED_SECONDS    99
#define TUNE_MAX_AMBER_SECONDS  98
#define TUNE_MAX_GREEN_SECONDS  98

#define TUNE_BLINK_HALF_MS      250U    /* Toggle every 250 ms -> 2 Hz blink */
#define TUNE_ERROR_SHOW_MS      1000U   /* How long TUNE_ERROR stays on screen */

/* Timing adjustment state machine runner.
 * Scheduled every 1 tick (10 ms). Only acts when status is TUNE_*. */
void fsm_tuning_run(void);

#endif /* INC_FSM_TUNING_H_ */
