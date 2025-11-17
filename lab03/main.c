/*
 * FreeRTOS Lab #3 - Variant 4 
 * Tasks: Producer, Consumer
 * Queue Data: char[16]
 */

/* --- 1. INCLUDES --- */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdint.h> 

/* Visual studio intrinsics */
#include <intrin.h>

#ifdef WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#else
#include <winsock.h>
#endif

/* FreeRTOS kernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "trcRecorder.h"

/* --- 2. DEFINES & CONSTANTS (CRITICAL FOR HEAP) --- */
#define mainREGION_1_SIZE   82010
#define mainREGION_2_SIZE   239050
#define mainREGION_3_SIZE   168070

/* Lab 3 Settings (Variant 4) */
#define QUEUE_LEN       10      // Queue length
#define PRODUCER_DELAY  200     // Producer delay (time between sends)
#define CONSUMER_DELAY  100     // Consumer delay (time between checks)

/* --- 3. DATA STRUCTURES --- */
// 'sensor_data_t' removed, as Variant 4 requires char[16] 

/* --- 4. GLOBAL VARIABLES --- */
QueueHandle_t xQueueData;

/* --- 5. FUNCTION PROTOTYPES --- */
void vProducer(void* arg);
void vConsumer(void* arg);
// vMonitor prototype removed

/* System Prototypes */
void vAssertCalled(unsigned long ulLine, const char* const pcFileName);
static void prvInitialiseHeap(void);
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName);
void vApplicationTickHook(void);
void vApplicationDaemonTaskStartupHook(void);

/* --- 6. TASKS IMPLEMENTATION --- */

/* Producer: Sends strings (Timeout 100ms) */
void vProducer(void* arg) {
    // Queue data type
    char data[16]; 
    uint32_t counter = 0;

    for (;;) {
        counter++;
        // Additional task: "send strings ('Msg1', 'Msg2')" 
        // We generate unique messages
        snprintf(data, 16, "Message %u", counter);

        /* Send timeout = 100 ms */
        if (xQueueSend(xQueueData, data, pdMS_TO_TICKS(100)) == pdPASS) {
            taskENTER_CRITICAL();
            printf("[Producer] Sent: %s\n", data);
            taskEXIT_CRITICAL();
        }
        else {
            taskENTER_CRITICAL();
            printf("[Producer] Queue FULL! Data dropped (%s)\n", data);
            taskEXIT_CRITICAL();
        }

        vTaskDelay(pdMS_TO_TICKS(PRODUCER_DELAY));
    }
}

/* Consumer: Receives strings (Timeout 1000ms) */
void vConsumer(void* arg) {
    // Queue data type
    char receivedData[16]; 

    for (;;) {
        /* Receive timeout = 1000 ms */
        if (xQueueReceive(xQueueData, receivedData, pdMS_TO_TICKS(1000)) == pdPASS) {
            taskENTER_CRITICAL();
            printf("    [Consumer] Processed: %s (Tick: %u)\n",
                   receivedData, xTaskGetTickCount());
            taskEXIT_CRITICAL();
        }
        else {
            taskENTER_CRITICAL();
            printf("    [Consumer] Timeout waiting for data...\n");
            taskEXIT_CRITICAL();
        }

        vTaskDelay(pdMS_TO_TICKS(CONSUMER_DELAY));
    }
}

/* --- 7. MAIN --- */
int main(void)
{
    /* Initialize Heap first */
    prvInitialiseHeap();

    /* Initialize Trace Recorder (Optional) */
    vTraceEnable(TRC_START);

    /* Create Queue */
    // Changed item size to sizeof(char[16]) as per Variant 4
    xQueueData = xQueueCreate(QUEUE_LEN, sizeof(char[16])); 

    if (xQueueData != NULL) {
        printf("\nStarting Lab 3 (Variant 4)...\n");

        /* Create Tasks */
        xTaskCreate(vProducer, "Producer", 1024, NULL, 1, NULL);
        xTaskCreate(vConsumer, "Consumer", 1024, NULL, 1, NULL);

        /* Start Scheduler */
        vTaskStartScheduler();
    }
    else {
        printf("Failed to create queue.\n");
    }

    return 0;
}

/* --- 8. HOOKS & HELPERS --- */

void vApplicationMallocFailedHook(void) { vAssertCalled(__LINE__, __FILE__); }
void vApplicationIdleHook(void) {}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName) { (void)pxTask; (void)pcTaskName; vAssertCalled(__LINE__, __FILE__); }
void vApplicationTickHook(void) {}
void vApplicationDaemonTaskStartupHook(void) {}

/* Static Allocation Helpers */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE* puxIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer, StackType_t** ppxTimerTaskStackBuffer, uint32_t* pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vAssertCalled(unsigned long ulLine, const char* const pcFileName) {
    printf("ASSERT! Line %ld, file %s\r\n", ulLine, pcFileName);
    __debugbreak();
    for (;; );
}

/* Heap Initialization (Uses defines from Section 2) */
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

/* Trace Recorder Timers */
static uint32_t ulEntryTime = 0;
void vTraceTimerReset(void) { ulEntryTime = xTaskGetTickCount(); }
uint32_t uiTraceTimerGetFrequency(void) { return configTICK_RATE_HZ; }
uint32_t uiTraceTimerGetValue(void) { return(xTaskGetTickCount() - ulEntryTime); }