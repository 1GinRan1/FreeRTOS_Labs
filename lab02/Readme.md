# RTOS Laboratory Work #2: Thread Local Storage (TLS)

This project demonstrates the use of **Thread Local Storage (TLS)** in FreeRTOS to create thread-safe tasks without using global variables. Each task maintains its own unique state and configuration using TLS pointers.

## Description

The goal of this laboratory work is to:
1.  Learn how to use TLS slots (`vTaskSetThreadLocalStoragePointer`, `xTaskGetThreadLocalStoragePointer`).
2.  Implement thread-safe modules where each task has its own context and execution profile.
3.  Avoid race conditions by eliminating shared global variables for task state.
4.  Implement a "Burst" logging mechanism to reduce console I/O overhead.

## Variant Details (Variant 4)

* **Student Variant:** 4
* **Total Tasks:** 4
* **Execution Mode:** `Delay`
* **Burst Size:** 4 (Logs are printed in batches of 4 lines)

### Task Configuration

| Parameter | Value | Description |
| :--- | :--- | :--- |
| **TLS Index 0** | `task_context_t` | Stores dynamic state (counters, checksums, buffers). |
| **TLS Index 1** | `profile_t` | Stores static configuration (mode, delays). |
| **Base Delay** | 130 ms | Starting delay for Task 0. |
| **Step** | 7 ms | Increment of delay for each subsequent task. |
| **Iterations** | 44 | Calculated as $N_i = 10 \times N_{tasks} + IDX$ ($10 \times 4 + 4$). |

## Implementation Details

### 1. Data Structures (TLS)
Instead of global variables, the following structures are allocated dynamically for each task:
* **Context (V1):** Contains the task name, current iteration `iter`, CRC8 `checksum`, and a string buffer `line1` for logging.
* **Profile (V2):** Contains execution settings like `baseDelayMs` and `burstMode`.

### 2. Logic Flow
1.  **Initialization:** The task allocates memory for Context and Profile, initializes them, and saves pointers to TLS slots (Index 0 and 1).
2.  **Loop:**
    * Increments iteration counter.
    * Updates CRC8 Checksum based on iteration and a unique seed.
    * Writes a log message to the internal buffer.
    * **Burst Check:** If the buffer has 4 lines, it flushes (prints) them to the console.
    * **Delay:** Waits for `130 + (7 * TaskID)` ms.
3.  **Cleanup:** Frees allocated memory and deletes the task upon completion.

## Results

The application successfully creates 4 independent tasks that run in parallel.
* **Thread Safety:** Each task calculates its own unique checksum and maintains its own iteration counter correctly, proving that TLS isolates data effectively.
* **Burst Logging:** The log output confirms that tasks buffer their log messages and print them in "bursts" of 4, as required.
* **Timing:** Tasks wake up at different intervals due to the calculated delay (`130ms`, `137ms`, `144ms`, and `151ms`).

### Actual Log Output (Variant 4)
```text
Starting Lab 2 (Variant 4)...
[Task T04_0] Tick: 2 Iter: 1 Sum: 0x00000072
[Task T04_0] Tick: 132 Iter: 2 Sum: 0x00000093
[Task T04_0] Tick: 262 Iter: 3 Sum: 0x000000CC
[Task T04_0] Tick: 392 Iter: 4 Sum: 0x0000004C
[Task T04_1] Tick: 2 Iter: 1 Sum: 0x000000EF
[Task T04_1] Tick: 139 Iter: 2 Sum: 0x0000000E
[Task T04_1] Tick: 276 Iter: 3 Sum: 0x00000051
[Task T04_1] Tick: 413 Iter: 4 Sum: 0x000000D1
[Task T04_2] Tick: 2 Iter: 1 Sum: 0x0000001B
[Task T04_2] Tick: 146 Iter: 2 Sum: 0x000000FA
[Task T04_2] Tick: 290 Iter: 3 Sum: 0x000000A5
[Task T04_2] Tick: 434 Iter: 4 Sum: 0x00000025
...