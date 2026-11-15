#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include "global.h"

void traffic_init(void);
void traffic_set(uint8_t road, TrafficColor_t color);
void traffic_blink_color(TrafficColor_t color);

#endif /* INC_TRAFFIC_LIGHT_H_ */
