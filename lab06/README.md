# FreeRTOS Lab 6: Stream & Message Buffers, Event Groups

## Description

This laboratory work demonstrates advanced Inter-Process Communication (IPC) mechanisms in FreeRTOS. The project simulates a **Network Gateway** system where raw data streams are received, parsed into packets, and routed to a destination.

**Key Concepts Covered:**
1.  **Stream Buffers:** For transferring a continuous stream of raw bytes (byte-oriented).
2.  **Message Buffers:** For transferring discrete, sized packets (message-oriented).
3.  **Event Groups:** For synchronizing tasks and signaling system states (errors, status flags).

---

## Variant Details (Variant 4)

* **Scenario:** Network Gateway Simulation.
* **Tasks:** 4 interacting tasks.
* **Data Flow:** Raw Bytes $\to$ Parsed Frames $\to$ Routed Packets.

### Task Roles

| Task Name | Priority | Role |
| :--- | :--- | :--- |
| **NetRxTask** | Low | **Producer.** Simulates a network interface. Generates raw bytes (with occasional errors) and sends them to a **Stream Buffer**. |
| **FrameBuilder** | Low | **Parser.** Reads the raw stream byte-by-byte. Detects Start/Stop markers, assembles frames, and sends complete packets to a **Message Buffer**. |
| **RouterTask** | Medium | **Consumer.** Reads complete packets from the Message Buffer and "routes" them (prints to console). |
| **MonitorTask** | High | **Supervisor.** Waits for **Event Bits** to report system status (Link Up, Errors, Overloads). |

---

## Architecture & IPC

The project uses three distinct IPC mechanisms to solve specific problems:

### 1. Stream Buffer (`xNetStreamBuf`)
* **Use Case:** Transferring raw data from `NetRx` to `FrameBuilder`.
* **Why:** Network data arrives as a continuous stream of bytes, not necessarily aligned to packet boundaries. Stream Buffers are optimized for this "byte-stream" access.

### 2. Message Buffer (`xFrameMsgBuf`)
* **Use Case:** Transferring assembled frames from `FrameBuilder` to `Router`.
* **Why:** Once a frame is built, it becomes a distinct unit (a message). Message Buffers preserve message boundaries and size, ensuring the Router gets one whole packet at a time.

### 3. Event Group (`xSystemEvents`)
* **Use Case:** Signaling events across tasks.
* **Flags:**
    * `EVT_LINK_UP`: Signal from NetRx that data is flowing.
    * `EVT_FRAME_ERROR`: Signal from FrameBuilder that a corrupted frame was detected (missing stop byte or unexpected start).
    * `EVT_ROUTER_BUSY`: Signal that the Message Buffer is full.

---

## Results Analysis

The application was successfully compiled and executed. The console output demonstrates the correct operation of the gateway pipeline.

### Log Output (Example)

```text
Starting Lab 6 (Variant 4: Network Gateway)...
[NetRx] Network Interface Started.
[Router] Packet Routed: Size=11 | Data: 094882455
[Router] Packet Routed: Size=8  | Data: 152761
[Router] Packet Routed: Size=10 | Data: 22168576
[Monitor] ALERT: Frame Integrity Error Detected!
[Router] Packet Routed: Size=16 | Data: 96378829135984
[Router] Packet Routed: Size=7  | Data: 41200
[Monitor] ALERT: Frame Integrity Error Detected!
[Router] Packet Routed: Size=10 | Data: 51201640