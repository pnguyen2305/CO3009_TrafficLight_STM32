#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "global.h"

extern uint8_t timer_flag[MAX_TIMERS];

void setTimer(uint8_t index, uint32_t duration_ms);
void clearTimer(uint8_t index);
void clearAllTimers(void);
void timer_run(void);

#endif /* INC_SOFTWARE_TIMER_H_ */
