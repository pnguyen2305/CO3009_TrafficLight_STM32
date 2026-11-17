#include "scheduler.h"
#include <stddef.h>

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint32_t SCH_count = 0U;

/* Internal helper: find an unused TaskID in [1 .. SCH_MAX_TASKS] */
static uint32_t sch_allocate_task_id(void) {
    for (uint32_t candidate = 1U; candidate <= SCH_MAX_TASKS; candidate++) {
        uint8_t in_use = 0U;
        for (uint32_t i = 0U; i < SCH_count; i++) {
            if (SCH_tasks_G[i].TaskID == candidate) {
                in_use = 1U;
                break;
            }
        }
        if (in_use == 0U) {
            return candidate;
        }
    }
    return NO_TASK_ID;
}

/* Internal helper: insert a task into the relative-delay sorted array */
static uint32_t sch_insert_with_id(void (*pTask)(void), uint32_t DELAY, uint32_t PERIOD, uint32_t taskID) {
    if ((pTask == NULL) || (SCH_count >= SCH_MAX_TASKS) || (taskID == NO_TASK_ID)) {
        return NO_TASK_ID;
    }

    uint32_t rel_delay = DELAY;
    uint32_t insert_idx = 0U;

    /* Find sorted insertion position and compute relative delay */
    while ((insert_idx < SCH_count) && (rel_delay >= SCH_tasks_G[insert_idx].Delay)) {
        rel_delay -= SCH_tasks_G[insert_idx].Delay;
        insert_idx++;
    }

    /* Shift downstream tasks right by 1 slot */
    for (uint32_t j = SCH_count; j > insert_idx; j--) {
        SCH_tasks_G[j] = SCH_tasks_G[j - 1U];
    }

    /* Deduct new task's relative delay from the task immediately following it */
    if (insert_idx < SCH_count) {
        SCH_tasks_G[insert_idx + 1U].Delay -= rel_delay;
    }

    /* Populate inserted task entry */
    SCH_tasks_G[insert_idx].pTask  = pTask;
    SCH_tasks_G[insert_idx].Delay  = rel_delay;
    SCH_tasks_G[insert_idx].Period = PERIOD;
    SCH_tasks_G[insert_idx].RunMe  = (rel_delay == 0U) ? 1U : 0U;
    SCH_tasks_G[insert_idx].TaskID = taskID;

    SCH_count++;
    return taskID;
}

/* Internal helper: remove element at index 0 after it becomes ready */
static void sch_pop_head(void) {
    if (SCH_count == 0U) {
        return;
    }

    for (uint32_t i = 0U; i < (SCH_count - 1U); i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1U];
    }

    SCH_count--;
    SCH_tasks_G[SCH_count].pTask  = NULL;
    SCH_tasks_G[SCH_count].Delay  = 0U;
    SCH_tasks_G[SCH_count].Period = 0U;
    SCH_tasks_G[SCH_count].RunMe  = 0U;
    SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;

    /* If the new head task has relative Delay == 0, mark it ready on the same tick */
    if ((SCH_count > 0U) && (SCH_tasks_G[0].Delay == 0U)) {
        SCH_tasks_G[0].RunMe = 1U;
    }
}

void SCH_Init(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    SCH_count = 0U;
    for (uint32_t i = 0U; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask  = NULL;
        SCH_tasks_G[i].Delay  = 0U;
        SCH_tasks_G[i].Period = 0U;
        SCH_tasks_G[i].RunMe  = 0U;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }

    if (primask == 0U) {
        __enable_irq();
    }
}

uint32_t SCH_Add_Task(void (*pTask)(void), uint32_t DELAY, uint32_t PERIOD) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t new_id = sch_allocate_task_id();
    uint32_t result = sch_insert_with_id(pTask, DELAY, PERIOD, new_id);

    if (primask == 0U) {
        __enable_irq();
    }
    return result;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID == NO_TASK_ID) {
        return 0U;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint8_t found = 0U;
    for (uint32_t i = 0U; i < SCH_count; i++) {
        if (SCH_tasks_G[i].TaskID == taskID) {
            /* Restore remaining relative delay to the next task to preserve timing */
            if ((i + 1U) < SCH_count) {
                SCH_tasks_G[i + 1U].Delay += SCH_tasks_G[i].Delay;
            }

            /* Shift remaining tasks left */
            for (uint32_t j = i; j < (SCH_count - 1U); j++) {
                SCH_tasks_G[j] = SCH_tasks_G[j + 1U];
            }

            SCH_count--;
            SCH_tasks_G[SCH_count].pTask  = NULL;
            SCH_tasks_G[SCH_count].Delay  = 0U;
            SCH_tasks_G[SCH_count].Period = 0U;
            SCH_tasks_G[SCH_count].RunMe  = 0U;
            SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;

            if ((i == 0U) && (SCH_count > 0U) && (SCH_tasks_G[0].Delay == 0U)) {
                SCH_tasks_G[0].RunMe = 1U;
            }

            found = 1U;
            break;
        }
    }

    if (primask == 0U) {
        __enable_irq();
    }
    return found;
}

/* Mandatory O(1) ISR update: only processes the head task SCH_tasks_G[0] */
void SCH_Update(void) {
    if (SCH_count == 0U) {
        return;
    }
    if (SCH_tasks_G[0].Delay > 0U) {
        SCH_tasks_G[0].Delay--;
    }
    if (SCH_tasks_G[0].Delay == 0U) {
        SCH_tasks_G[0].RunMe = 1U;
    }
}

void SCH_Dispatch_Tasks(void) {
    while (1) {
        sTask ready_task;
        uint8_t has_ready = 0U;

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        if ((SCH_count > 0U) && (SCH_tasks_G[0].RunMe > 0U)) {
            ready_task = SCH_tasks_G[0];
            sch_pop_head();

            /* Re-schedule periodic tasks with the same TaskID; one-shot (Period == 0) is dropped */
            if (ready_task.Period > 0U) {
                (void)sch_insert_with_id(ready_task.pTask, ready_task.Period, ready_task.Period, ready_task.TaskID);
            }
            has_ready = 1U;
        }

        if (primask == 0U) {
            __enable_irq();
        }

        if (has_ready == 0U) {
            break;
        }

        /* Execute task callback outside critical section */
        if (ready_task.pTask != NULL) {
            (*ready_task.pTask)();
        }
    }
}