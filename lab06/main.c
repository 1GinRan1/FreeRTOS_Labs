/*
 * FreeRTOS Lab #6 - Variant 4: Network Gateway Simulation
 *
 * SCENARIO:
 * 1. NetRxTask (Producer) -> Stream Buffer (Raw Bytes)
 * 2. FrameBuilderTask (Parser) -> Message Buffer (Packets)
 * 3. RouterTask (Consumer) -> Console
 * 4. MonitorTask -> Event Group (Status flags)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "message_buffer.h"
#include "event_groups.h"

/* --- CONFIGURATION --- */
#define STREAM_BUFFER_SIZE_BYTES    128     /* Capacity of the raw byte stream buffer */
#define MESSAGE_BUFFER_SIZE_BYTES   256     /* Capacity of the packet message buffer */
#define FRAME_MAX_LEN               64      /* Maximum allowed length of a frame */

/* Frame delimiters */
#define FRAME_START_BYTE            0xAA
#define FRAME_STOP_BYTE             0x55

/* Event Group Bits */
#define EVT_LINK_UP                 (1 << 0) /* Bit 0: Network traffic detected */
#define EVT_FRAME_ERROR             (1 << 1) /* Bit 1: Malformed frame detected */
#define EVT_ROUTER_BUSY             (1 << 2) /* Bit 2: Router queue is full */

/* Task Priorities */
#define PRIO_NET_RX                 (tskIDLE_PRIORITY + 2)
#define PRIO_BUILDER                (tskIDLE_PRIORITY + 1)
#define PRIO_ROUTER                 (tskIDLE_PRIORITY + 2)
#define PRIO_MONITOR                (tskIDLE_PRIORITY + 3)

/* --- HANDLES --- */
static StreamBufferHandle_t xNetStreamBuf = NULL;
static MessageBufferHandle_t xFrameMsgBuf = NULL;
static EventGroupHandle_t xSystemEvents = NULL;

/* --- MEMORY ALLOCATION (HEAP_5) --- */
#define mainREGION_1_SIZE           82010
#define mainREGION_2_SIZE           239050
#define mainREGION_3_SIZE           168070

/* --- HELPER FUNCTIONS --- */
static void prvInitialiseHeap(void);

/* Generates a random frame: [START] [DATA...] [STOP] */
void vGenerateRandomPacket(uint8_t *buffer, size_t *len, uint8_t corrupt) {
    size_t payload_len = 5 + (rand() % 15); // Payload length between 5 and 20 bytes
    size_t i;
    
    buffer[0] = FRAME_START_BYTE;
    
    for (i = 0; i < payload_len; i++) {
        buffer[i + 1] = 0x30 + (rand() % 10); // Random ASCII digits (0-9)
    }
    
    if (corrupt) {
        /* Simulate error: missing stop byte or wrong byte */
        buffer[payload_len + 1] = 0xFF; 
    } else {
        buffer[payload_len + 1] = FRAME_STOP_BYTE;
    }
    
    *len = payload_len + 2; // Total length: START + PAYLOAD + STOP
}

/* --- TASKS --- */

/* 1. NetRxTask: Simulates receiving raw bytes from a network interface */
void vNetRxTask(void *pvParameters) {
    uint8_t txBuffer[FRAME_MAX_LEN];
    size_t txLen;
    size_t sentBytes;
    TickType_t xDelay;

    printf("[NetRx] Network Interface Started.\n");

    for (;;) {
        /* Random delay to simulate network traffic intervals */
        xDelay = pdMS_TO_TICKS(200 + (rand() % 800));
        vTaskDelay(xDelay);

        /* Generate a frame (10% chance of corruption to test error handling) */
        uint8_t is_corrupted = ((rand() % 10) == 0);
        vGenerateRandomPacket(txBuffer, &txLen, is_corrupted);

        /* Send RAW bytes to Stream Buffer */
        sentBytes = xStreamBufferSend(xNetStreamBuf, txBuffer, txLen, 0);

        if (sentBytes != txLen) {
            printf("[NetRx] ERROR: Stream Buffer Full!\n");
        } else {
            /* Signal that the link is active (LINK_UP) */
            xEventGroupSetBits(xSystemEvents, EVT_LINK_UP);
            // printf("[NetRx] Received %d bytes (Raw Stream)\n", (int)txLen);
        }
    }
}

/* 2. FrameBuilderTask: Parses the raw stream and reconstructs frames */
void vFrameBuilderTask(void *pvParameters) {
    uint8_t rxByte;
    uint8_t frameBuffer[FRAME_MAX_LEN];
    size_t frameIndex = 0;
    uint8_t isCollecting = 0;
    size_t xReceivedBytes;

    for (;;) {
        /* Read ONE byte at a time from Stream Buffer (blocking wait) */
        xReceivedBytes = xStreamBufferReceive(xNetStreamBuf, &rxByte, 1, portMAX_DELAY);

        if (xReceivedBytes > 0) {
            if (!isCollecting) {
                /* Looking for the START byte */
                if (rxByte == FRAME_START_BYTE) {
                    isCollecting = 1;
                    frameIndex = 0;
                    frameBuffer[frameIndex++] = rxByte;
                }
            } else {
                /* Collecting frame body */
                if (frameIndex < FRAME_MAX_LEN) {
                    frameBuffer[frameIndex++] = rxByte;

                    if (rxByte == FRAME_STOP_BYTE) {
                        /* End found! Send the complete packet to Message Buffer */
                        size_t sent = xMessageBufferSend(xFrameMsgBuf, frameBuffer, frameIndex, 0);
                        
                        if (sent == 0) {
                            /* Message Buffer is full -> Router is busy */
                            xEventGroupSetBits(xSystemEvents, EVT_ROUTER_BUSY);
                        }

                        isCollecting = 0; // Reset state
                    } 
                    else if (rxByte == FRAME_START_BYTE) {
                        /* Error: New START byte found inside an incomplete frame */
                        xEventGroupSetBits(xSystemEvents, EVT_FRAME_ERROR);
                        frameIndex = 0; 
                        frameBuffer[frameIndex++] = rxByte; // Start capturing the new frame
                    }
                } else {
                    /* Buffer overflow (Frame too long without a STOP byte) */
                    xEventGroupSetBits(xSystemEvents, EVT_FRAME_ERROR);
                    isCollecting = 0;
                }
            }
        }
    }
}

/* 3. RouterTask: Routes the completed packets */
void vRouterTask(void *pvParameters) {
    uint8_t msgBuffer[FRAME_MAX_LEN];
    size_t msgLen;

    for (;;) {
        /* Wait for a complete message from Message Buffer */
        msgLen = xMessageBufferReceive(xFrameMsgBuf, msgBuffer, sizeof(msgBuffer), portMAX_DELAY);

        if (msgLen > 0) {
            /* Simulate packet processing */
            printf("[Router] Packet Routed: Size=%d | Data: ", (int)msgLen);
            
            /* Print payload (skipping START and STOP bytes) */
            for(size_t i = 1; i < msgLen - 1; i++) {
                putchar(msgBuffer[i]);
            }
            printf("\n");
        }
    }
}

/* 4. MonitorTask: Monitors system status events */
void vMonitorTask(void *pvParameters) {
    EventBits_t uxBits;

    for (;;) {
        /* Wait for ANY relevant event bit (LINK_UP, ERROR, BUSY) */
        uxBits = xEventGroupWaitBits(
            xSystemEvents,
            EVT_LINK_UP | EVT_FRAME_ERROR | EVT_ROUTER_BUSY,
            pdTRUE,  /* Clear bits on exit */
            pdFALSE, /* Wait for ANY bit (OR), not ALL (AND) */
            portMAX_DELAY
        );

        if ((uxBits & EVT_LINK_UP) != 0) {
            /* Link is up, traffic is flowing. Optional: Blink an LED */
        }

        if ((uxBits & EVT_FRAME_ERROR) != 0) {
            printf("[Monitor] ALERT: Frame Integrity Error Detected!\n");
        }

        if ((uxBits & EVT_ROUTER_BUSY) != 0) {
            printf("[Monitor] WARNING: Router is overloaded!\n");
        }
    }
}


/* --- MAIN FUNCTION --- */
int main(void) {
    /* 1. Initialize Heap Memory */
    prvInitialiseHeap();
    vTraceEnable(TRC_START);

    /* 2. Create IPC Objects */
    /* Stream Buffer: 128 bytes, trigger level 1 byte */
    xNetStreamBuf = xStreamBufferCreate(STREAM_BUFFER_SIZE_BYTES, 1);
    
    /* Message Buffer: 256 bytes */
    xFrameMsgBuf = xMessageBufferCreate(MESSAGE_BUFFER_SIZE_BYTES);
    
    /* Event Group */
    xSystemEvents = xEventGroupCreate();

    printf("\nStarting Lab 6 (Variant 4: Network Gateway)...\n");

    /* 3. Create Tasks */
    xTaskCreate(vNetRxTask,         "NetRx",    1024, NULL, PRIO_NET_RX,    NULL);
    xTaskCreate(vFrameBuilderTask,  "Builder",  1024, NULL, PRIO_BUILDER,   NULL);
    xTaskCreate(vRouterTask,        "Router",   1024, NULL, PRIO_ROUTER,    NULL);
    xTaskCreate(vMonitorTask,       "Monitor",  1024, NULL, PRIO_MONITOR,   NULL);

    /* 4. Start Scheduler */
    vTaskStartScheduler();

    return 0;
}

/* --- REQUIRED HOOKS & SETUP (DO NOT CHANGE) --- */

void vAssertCalled(unsigned long ulLine, const char* const pcFileName) {
    printf("ASSERT! Line %ld, file %s\r\n", ulLine, pcFileName);
    for (;; );
}

void vApplicationMallocFailedHook(void) { vAssertCalled(__LINE__, __FILE__); }
void vApplicationIdleHook(void) {}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName) { (void)pxTask; (void)pcTaskName; vAssertCalled(__LINE__, __FILE__); }
void vApplicationTickHook(void) {}
void vApplicationDaemonTaskStartupHook(void) {}

static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE* puxIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB;
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer, StackType_t** ppxTimerTaskStackBuffer, uint32_t* pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

static void prvInitialiseHeap(void) {
    static uint8_t ucHeap[configTOTAL_HEAP_SIZE];
    const HeapRegion_t xHeapRegions[] = {
        { ucHeap + 1, mainREGION_1_SIZE },
        { ucHeap + 15 + mainREGION_1_SIZE, mainREGION_2_SIZE },
        { ucHeap + 19 + mainREGION_1_SIZE + mainREGION_2_SIZE, mainREGION_3_SIZE },
        { NULL, 0 }
    };
    vPortDefineHeapRegions(xHeapRegions);
}

/* Trace hooks */
static uint32_t ulEntryTime = 0;
void vTraceTimerReset(void) { ulEntryTime = xTaskGetTickCount(); }
uint32_t uiTraceTimerGetFrequency(void) { return configTICK_RATE_HZ; }
uint32_t uiTraceTimerGetValue(void) { return(xTaskGetTickCount() - ulEntryTime); }