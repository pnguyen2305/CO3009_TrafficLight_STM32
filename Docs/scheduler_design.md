# O(1) Cooperative Scheduler Design & Complexity Analysis

> **Owner:** Huy | **Module:** `Core/Inc/scheduler.h`, `Core/Src/scheduler.c` | **Milestone:** `m2-scheduler-hal`

## 1. Data Structure Selection: Sorted Array vs. Linked List

| Criterion | Sorted Static Array (`SCH_tasks_G[16]`) | Dynamic Singly Linked List |
|---|---|---|
| **Memory Allocation** | Static compile-time allocation (`~256 bytes` in `.bss`), zero heap fragmentation | Requires dynamic `malloc()`/`free()` or a custom node pool |
| **Cache & Access Overhead** | Contiguous memory, direct index access `SCH_tasks_G[0]` | Pointer indirection overhead on every traversal |
| **`SCH_Update()` ISR Time** | **Strict $O(1)$** (touches only `SCH_tasks_G[0]`) | Strict $O(1)$ (touches only `head`) |
| **`SCH_Add_Task()` Time** | $O(N)$ search and shift ($N \le 16$, `< 2 us` at `64 MHz`) | $O(N)$ pointer traversal |

**Justification:** On the `STM32F103RB` (`20 KB` SRAM) with at most `8–10` active tasks, a statically allocated sorted array (`SCH_MAX_TASKS = 16`) eliminates heap fragmentation risks while guaranteeing deterministic $O(1)$ interrupt execution.

## 2. Relative-Delay Queue Invariant

Suppose active tasks $T_0, T_1, \dots, T_{k-1}$ are scheduled to execute at absolute tick offsets $A_0 \le A_1 \le \dots \le A_{k-1}$ from the current tick. The queue stores:
- $D_0 = A_0$
- $D_i = A_i - A_{i-1}$ for all $1 \le i < k$

Consequently, the absolute remaining time for any task $T_m$ is the prefix sum $\sum_{i=0}^{m} D_i$.

1. **`SCH_Update()` ($O(1)$):** Every `10 ms` (`TIM2` interrupt), decrementing $D_0$ by `1` implicitly decrements the remaining time of **every** task in the queue by `1` tick. When $D_0 = 0$, `SCH_tasks_G[0].RunMe` is set to `1`.
2. **`SCH_Add_Task()` ($O(N)$):** Subtracts $D_i$ sequentially while `rel_delay >= D_i` to locate insertion index `idx`, shifts downstream tasks right by one slot, and subtracts the residual `rel_delay` from `SCH_tasks_G[idx + 1].Delay`.
3. **`SCH_Delete_Task()` ($O(N)$):** Before shifting elements left to overwrite index `i`, adds `SCH_tasks_G[i].Delay` to `SCH_tasks_G[i + 1].Delay` so downstream prefix sums remain invariant.
4. **Simultaneous Zero-Delay & One-Shot Tasks (`SCH_Dispatch_Tasks()`):**
   - When the head task is popped (`sch_pop_head()`), if the new head task has `Delay == 0`, its `RunMe` flag is immediately asserted so all tasks due on the same tick execute in a single pass.
   - If `Period > 0`, the task is re-inserted with `Delay = Period` while preserving its `TaskID`. If `Period == 0` (one-shot task), it is not re-inserted.