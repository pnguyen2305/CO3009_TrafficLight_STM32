#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include "main.h"
#include <stdint.h>

#define SCH_MAX_TASKS           16U
#define NO_TASK_ID              0U

typedef struct {
    void (*pTask)(void);    // Pointer to the task function
    uint32_t Delay;         // Remaining ticks (relative to previous task)
    uint32_t Period;        // Period in ticks (0 = one-shot task)
    uint8_t RunMe;          // Flag: task ready to run
    uint32_t TaskID;        // Unique non-zero task identifier
} sTask;

void SCH_Init(void);
uint32_t SCH_Add_Task(void (*pTask)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);

#endif /* INC_SCHEDULER_H_ */