/*
 * FreeRTOS Lab #5 - Variant 4
 * Topic: Direct-to-Task Notifications & Software Timers
 * Scenario: Client-Server Simulation with Timeout Monitoring
 *
 * Tasks:
 * 1. ClientTask: Generates requests periodically.
 * 2. ServerTask: Processes requests with simulated variable delay.
 * 3. MonitorTask: Counts processed requests and logs Timeouts.
 *
 * Timers:
 * 1. Periodic Timer: Triggers the Client to send a new request.
 * 2. One-Shot Timer: Watchdog/Timeout timer. Fires if Server takes too long.
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>

#include <intrin.h>

#ifdef WIN32_LEAN_AND_MEAN
    #include "winsock2.h"
#else
    #include <winsock.h>
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h" 
#include "semphr.h"
#include "trcRecorder.h"

#define mainREGION_1_SIZE      82010
#define mainREGION_2_SIZE      239050
#define mainREGION_3_SIZE      168070

/* --- CONFIGURATION --- */
#define REQUEST_GEN_PERIOD_MS   2000    // How often Client sends a request
#define RESPONSE_TIMEOUT_MS     1500    // Max allowed time for Server to respond
#define SERVER_MIN_DELAY        500     // Min processing time simulation
#define SERVER_MAX_DELAY        2500    // Max processing time simulation

/* Notification Bits for MonitorTask */
#define NOTIFY_SUCCESS          (1 << 0) // Bit 0: Success
#define NOTIFY_TIMEOUT          (1 << 1) // Bit 1: Timeout

/* --- GLOBALS (HANDLES) --- */
TaskHandle_t xClientHandle = NULL;
TaskHandle_t xServerHandle = NULL;
TaskHandle_t xMonitorHandle = NULL;

TimerHandle_t xPeriodicTimer = NULL;
TimerHandle_t xTimeoutTimer = NULL;

/* --- FUNCTION PROTOTYPES --- */
void vClientTask(void *pvParameters);
void vServerTask(void *pvParameters);
void vMonitorTask(void *pvParameters);
void vPeriodicTimerCallback(TimerHandle_t xTimer);
void vTimeoutTimerCallback(TimerHandle_t xTimer);

static void prvInitialiseHeap( void );
void vAssertCalled( unsigned long ulLine, const char * const pcFileName );

/* --- TIMER CALLBACKS --- */

/* 1. Periodic Timer Callback: Wakes up the ClientTask */
void vPeriodicTimerCallback(TimerHandle_t xTimer) {
    xTaskNotifyGive(xClientHandle);
}

/* 2. One-Shot Timer Callback: Handles Response Timeout */
void vTimeoutTimerCallback(TimerHandle_t xTimer) {/
    xTaskNotify(xMonitorHandle, NOTIFY_TIMEOUT, eSetBits);
}

/* --- TASKS --- */

/* ClientTask: Generates requests */
void vClientTask(void *pvParameters) {
    uint32_t ulRequestID = 1;

    for (;;) {
        /* Wait for the Periodic Timer to wake us up */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("\n[Client] >>> Generating Request #%u...\n", ulRequestID);

        xTimerStart(xTimeoutTimer, 0);

        xTaskNotify(xServerHandle, ulRequestID, eSetValueWithOverwrite);

        ulRequestID++;
    }
}

/* ServerTask: Processes requests */
void vServerTask(void *pvParameters) {
    uint32_t ulReceivedID;
    
    /* Processing delay variable */
    int processingTime; 

    for (;;) {
        /* Wait for a request from Client (receive value in ulReceivedID) */
        if (xTaskNotifyWait(0, 0, &ulReceivedID, portMAX_DELAY) == pdTRUE) {
            
            printf("    [Server] Processing Request #%u...\n", ulReceivedID);

            /* Simulate processing time (Random between MIN and MAX) */
            processingTime = SERVER_MIN_DELAY + (rand() % (SERVER_MAX_DELAY - SERVER_MIN_DELAY));
            vTaskDelay(pdMS_TO_TICKS(processingTime));

            if (xTimerStop(xTimeoutTimer, 0) == pdPASS) {
                /* SUCCESS CASE: Timer was running, so we stopped it before timeout. */
                printf("    [Server] Done in %d ms. Sending SUCCESS to Monitor.\n", processingTime);
                
                xTaskNotify(xMonitorHandle, NOTIFY_SUCCESS, eSetBits);
            } 
            else {
                printf("    [Server] Done in %d ms, but TOO LATE (Timeout was %d ms).\n", processingTime, RESPONSE_TIMEOUT_MS);
            }
        }
    }
}

/* MonitorTask: Tracks statistics */
void vMonitorTask(void *pvParameters) {
    uint32_t ulNotificationValue;
    uint32_t ulTotalRequests = 0;
    uint32_t ulTimeouts = 0;

    for (;;) {
        /* Wait for notification from Server (Success) or Timer (Timeout) 
           0xFFFFFFFF clears all bits on exit */
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) == pdTRUE) {
            
            ulTotalRequests++;

            if ((ulNotificationValue & NOTIFY_SUCCESS) != 0) {
                printf("        [Monitor] Status: OK. Total Processed: %u\n", ulTotalRequests);
            }
            
            if ((ulNotificationValue & NOTIFY_TIMEOUT) != 0) {
                ulTimeouts++;
                printf("        [Monitor] Status: TIMEOUT ALERT! Total Timeouts: %u\n", ulTimeouts);
            }
        }
    }
}

/* --- MAIN --- */
int main(void) {
    /* Initialize Heap and Trace Recorder */
    prvInitialiseHeap();
    vTraceEnable(TRC_START);
    
    /* Seed random generator */
    srand(xTaskGetTickCount()); 

    printf("--- Lab 5 Variant 4: Client-Server Simulation ---\n");
    printf("--- Period: %dms | Timeout: %dms ---\n", REQUEST_GEN_PERIOD_MS, RESPONSE_TIMEOUT_MS);

    /* 1. Create Tasks */
    xTaskCreate(vClientTask,  "Client",  1024, NULL, 1, &xClientHandle);
    xTaskCreate(vServerTask,  "Server",  1024, NULL, 2, &xServerHandle); 
    xTaskCreate(vMonitorTask, "Monitor", 1024, NULL, 3, &xMonitorHandle);

    /* 2. Create Software Timers */
    
    /* Periodic Timer: "Background Client" generator */
    xPeriodicTimer = xTimerCreate("AutoReqTimer", 
                                  pdMS_TO_TICKS(REQUEST_GEN_PERIOD_MS), 
                                  pdTRUE,  // Auto-reload = TRUE (Periodic)
                                  (void*)0, 
                                  vPeriodicTimerCallback);

    /* One-Shot Timer: Response Timeout Watchdog */
    xTimeoutTimer = xTimerCreate("TimeoutTimer", 
                                 pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS), 
                                 pdFALSE, // Auto-reload = FALSE (One-shot)
                                 (void*)1, 
                                 vTimeoutTimerCallback);

    if ((xPeriodicTimer != NULL) && (xTimeoutTimer != NULL)) {
        /* Start the Periodic Timer to kick off the simulation */
        xTimerStart(xPeriodicTimer, 0);

        vTaskStartScheduler();
    }
    else {
        printf("Failed to create timers.\n");
    }

    /* Infinite loop if scheduler fails */
    for (;;);
    return 0;
}


/* 1. Trace Recorder Hooks */
static uint32_t ulEntryTime = 0;

void vTraceTimerReset( void ) {
    ulEntryTime = xTaskGetTickCount();
}

uint32_t uiTraceTimerGetFrequency( void ) {
    return configTICK_RATE_HZ;
}

uint32_t uiTraceTimerGetValue( void ) {
    return( xTaskGetTickCount() - ulEntryTime );
}

/* 2. Static Memory Allocation for Idle and Timer Tasks */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, 
                                    StackType_t ** ppxIdleTaskStackBuffer, 
                                    configSTACK_DEPTH_TYPE * puxIdleTaskStackSize ) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* Required for Software Timers (configUSE_TIMERS = 1) */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, 
                                     StackType_t ** ppxTimerTaskStackBuffer, 
                                     uint32_t * pulTimerTaskStackSize ) {
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/* 3. General Hooks */
void vApplicationDaemonTaskStartupHook( void ) { }
void vApplicationIdleHook( void ) { }
void vApplicationTickHook( void ) { }

void vApplicationMallocFailedHook( void ) { 
    vAssertCalled( __LINE__, __FILE__ ); 
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char * pcTaskName ) { 
    (void)pxTask; (void)pcTaskName; 
    vAssertCalled( __LINE__, __FILE__ ); 
}

void vAssertCalled( unsigned long ulLine, const char * const pcFileName ) {
    printf( "ASSERT! Line %ld, file %s\r\n", ulLine, pcFileName );
    __debugbreak();
    for( ;; );
}

static void prvInitialiseHeap( void ) {
    static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
    const HeapRegion_t xHeapRegions[] = {
        { ucHeap + 1, mainREGION_1_SIZE },
        { ucHeap + 15 + mainREGION_1_SIZE, mainREGION_2_SIZE },
        { ucHeap + 19 + mainREGION_1_SIZE + mainREGION_2_SIZE, mainREGION_3_SIZE },
        { NULL, 0 }
    };
    vPortDefineHeapRegions( xHeapRegions );
}