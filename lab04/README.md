# RTOS Laboratory Work #4 - Variant 4

This project demonstrates resource management and task synchronization using a **Counting Semaphore** in FreeRTOS.

---

## Student Information
* **Variant:** 4
* **Mechanism:** Counting Semaphore 
* **Tasks:** 4 (Client 1, Client 2, Client 3, Client 4)
* **Resource Limit:** 2 Washing Machines
* **Platform:** FreeRTOS Windows Simulator (MSVC)

---

## Task Description: Washing Machines Pool (Counting Semaphore)

The goal is to simulate **four client tasks** accessing a limited pool of **two washing machines** concurrently. The **Counting Semaphore** mechanism is used to ensure that no more than two clients can occupy a machine at any given time.

1.  **Counting Semaphore (`xCountingSemaphore`):**
    * Created using `xSemaphoreCreateCounting(2, 2)`.
    * `Max_Count` is set to **2** (number of available machines).
    * `Initial_Count` is set to **2** (initially both machines are free).
    * It acts as a counter for available resources.

2.  **Tasks: Client 1 - Client 4:**
    * Each client task represents a customer attempting to use a washing machine.
    * A client first calls `xSemaphoreTake(..., portMAX_DELAY)` to **acquire a machine slot**.
    * If the counter is zero, the client **blocks (WAITS)** until a slot is released.
    * **Resource Usage Simulation:** If successful, the client performs a delay of **1200 ms** (`vTaskDelay(1200ms)`) to simulate the washing process.
    * After the delay, the client calls `xSemaphoreGive()` to **release the machine slot**, allowing a waiting client to proceed.

---

## How to Run

1.  Open the FreeRTOS solution (`RTOSDemo.sln`) in Visual Studio.
2.  Ensure that the project's `main.c` file contains the code for Lab 4, Variant 4.
3.  Exclude any conflicting `main.c` files from the build configuration.
4.  Build the project and run it (F5).

## Expected Output

The console output must clearly show:
1.  Only two clients acquiring a washer simultaneously (e.g., Client1 and Client2)[cite: 4].
2.  The remaining clients (e.g., Client3 and Client4) entering a **`WAITING... All 2 washers are busy`** state.
3.  The waiting clients immediately acquiring a washer slot once one of the initial clients releases the resource.