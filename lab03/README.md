# RTOS Laboratory Work #3 - Variant 4

This project demonstrates inter-task communication in FreeRTOS using Queues.

## Student Information
* **Variant:** 4
* **Number of Tasks:** 2 (Producer, Consumer)
* **Platform:** FreeRTOS Windows Simulator (MSVC)

## Task Description
1.  **Queue Configuration:**
    * [cite_start]Type: `char[16]` (string) [cite: 45]
    * Length: 10 items (as per default lab setup)

2.  **Producer Task:**
    * [cite_start]Generates string messages (e.g., "Message 1", "Message 2")[cite: 45].
    * [cite_start]Sends data to the queue with a **100ms timeout**[cite: 45].
    * If the queue is full after 100ms, a "Queue FULL" warning is printed.

3.  **Consumer Task:**
    * [cite_start]Waits for data from the queue with a **1000ms timeout**[cite: 45].
    * Prints the received string or a timeout message if no data arrives within 1000ms.

4.  **Additional Task (Variant 4):**
    * The core task is to successfully pass string messages between the tasks, which is integrated into the Producer/Consumer logic.

## How to Run
1.  Open `RTOSDemo.sln` in Visual Studio.
2.  Ensure your `main.c` contains the Variant 4 code (Lab 3).
3.  Exclude other main files (like `main_blinky.c`, `main_full.c`) from the build.
4.  Build and Run (F5).

## Expected Output
The console will show:
-   Producer sending messages (e.g., "[Producer] Sent: Message 1").
-   Consumer receiving and processing them (e.g., "[Consumer] Processed: Message 1 (Tick: ...)")
-   "Queue FULL" messages if the Producer is significantly faster than the Consumer (controlled by delays).
-   "Timeout waiting for data..." messages if the Consumer is significantly faster than the Producer.