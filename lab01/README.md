Lab 1: Introduction to FreeRTOS

## Description

This lab covers the basics of FreeRTOS by creating, running, and monitoring three periodic tasks. The primary goals were:
1.  To create three periodic tasks ($\t_1, \t_2, \t_3$) based on a calculated variant.
2.  To implement a "busy-wait" loop to simulate computational work ($C_i$).
3.  To log start and finish timestamps (in ticks) for each task.
4.  To observe and confirm priority-based preemption.

## Variant Details (ID = 4)

Based on the student ID `4`, the following parameters were calculated:

* **Prefix:** `VOME_`
* **Periods:**
    * T1 = 17 ms
    * T2 = 25 ms
    * T3 = 38 ms
* **Compute Times:**
    * C1 = 3 ms
    * C2 = 4 ms
    * C3 = 4 ms
* **Priorities:**
    * Rule: $\t_1 > \t_2 > \t_3$ (since ID is even)
    * `VOME_Task1`: Priority 3 (Highest)
    * `VOME_Task2`: Priority 2
    * `VOME_Task3`: Priority 1 (Lowest)

---

## Results

The program was successfully compiled and executed. The console log shows all three tasks running periodically according to their specified parameters.

The key result is the clear demonstration of **priority-based preemption**. The log shows that the higher-priority task (`VOME_Task1`) correctly interrupts (preempts) the lower-priority task (`VOME_Task2`) as soon as it becomes ready.

### Log Snippet: Preemption Example

This snippet shows the scheduler preempting `Task2` in favor of `Task1`.