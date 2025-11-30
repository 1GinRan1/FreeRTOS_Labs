# RTOS Laboratory Work #5 - Variant 4

This project demonstrates the use of **Direct-to-Task Notifications** and **Software Timers** in FreeRTOS. The scenario simulates a **Client-Server architecture** with a watchdog mechanism to detect processing delays.

---

## 👩‍💻 Student Information
* **Variant:** 4
* **Scenario:** Simulation of "Client-Server" request processing.
* **Key Mechanisms:** `xTaskNotify`, `xTimerCreate` (Periodic & One-shot).

---

## ⚙️ Project Description

The system consists of three tasks interacting via notifications and two software timers controlling the workflow.

### 1. Task Logic
* **ClientTask:** Acts as the request generator. It waits for a signal from the *Periodic Timer*. When triggered, it:
    * Starts a **One-Shot Timer** (Watchdog) set to 1500ms.
    * Sends a request ID to the Server using `xTaskNotify` (sending data as a *value*).
* **ServerTask:** Receives the request ID and simulates processing with a random delay (between 500ms and 2500ms).
    * If processing finishes **before** the timeout, it notifies the Monitor of **SUCCESS**.
* **MonitorTask:** Central logging task. It waits for notifications from either the Server (Success) or the Timer callback (Timeout) and updates global statistics.

### 2. Software Timers
* **AutoRequestTimer (Periodic):** Fires every **2000 ms**. It wakes up the `ClientTask` to initiate a new request cycle.
* **TimeoutTimer (One-Shot):** Set to **1500 ms**. It is started when a request is sent. If the Server fails to respond in time, the timer's callback sends a `TIMEOUT` notification to the `MonitorTask`.

---

## 🚀 How to Run

1.  Open the FreeRTOS solution (`RTOSDemo.sln`) in Visual Studio.
2.  Ensure `main.c` contains the Lab 5 code.
3.  Make sure `configUSE_TIMERS` is set to `1` in `FreeRTOSConfig.h`.
4.  Build and Run (F5).

## ✅ Expected Output

The console output demonstrates normal operation and timeout detection:

1.  **Normal Case:** Processing time < 1500ms. Status is "OK".
2.  **Timeout Case:** Processing time > 1500ms. The Monitor reports a "TIMEOUT ALERT".

```text
[Client] >>> Generating Request #1...
    [Server] Processing Request #1...
    [Server] Done in 541 ms. Sending SUCCESS to Monitor.
        [Monitor] Status: OK. Total Processed: 1

...

[Client] >>> Generating Request #5...
    [Server] Processing Request #5...
        [Monitor] Status: TIMEOUT ALERT! Total Timeouts: 1
    [Server] Done in 1669 ms. Sending SUCCESS to Monitor.