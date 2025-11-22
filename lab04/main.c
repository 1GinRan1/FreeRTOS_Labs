/*
 * FreeRTOS Lab #4 - Variant 4
 * Counting Semaphore - Limited Resource Pool (Washing Machines)
 * Resources: 2 Washing Machines
 * Tasks: 4 Clients (Client1, Client2, Client3, Client4)
 */

/* --- INCLUDES --- */
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

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" /* Required for Semaphores */
#include "trcRecorder.h"

/* --- HEAP CONFIGURATION --- */
#define mainREGION_1_SIZE      82010
#define mainREGION_2_SIZE      239050
#define mainREGION_3_SIZE      168070

/* --- CONFIGURATION --- */
#define MAX_WASHERS             2    // Maximum number of available washers
#define NUM_CLIENTS             4    // Number of client tasks
#define WASHING_TIME_MS      1200    // Simulation time for washing

/* --- GLOBALS --- */
/* Main synchronization object - Counting Semaphore */
SemaphoreHandle_t xCountingSemaphore;

/* --- FUNCTION PROTOTYPES --- */
void vClient(void *pvParameters);

/* System Prototypes */
void vAssertCalled( unsigned long ulLine, const char * const pcFileName );
static void prvInitialiseHeap( void );
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char * pcTaskName );
void vApplicationTickHook( void );
void vApplicationDaemonTaskStartupHook( void );

/* --- TASKS --- */

/* Client Task: Tries to get a washing machine slot */
void vClient(void *pvParameters) {
    const char *pcTaskName = pcTaskGetName(NULL);
    const TickType_t xRandomDelay = pdMS_TO_TICKS((rand() % 500) + 100); // Small random offset for varied arrival times

    vTaskDelay(xRandomDelay);

    for (;;) {
        printf("[%s] ARRIVED at Tick: %u. Trying to get a washer...\n", pcTaskName, xTaskGetTickCount());

        /* 1. Try to take the semaphore (acquire a washer) */
        if (xSemaphoreTake(xCountingSemaphore, (TickType_t)0) == pdFALSE) {
             printf("--- [%s] WAITING... All %d washers are busy at Tick: %u ---\n", pcTaskName, MAX_WASHERS, xTaskGetTickCount());

             /* If failed to take immediately, wait indefinitely */
             xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);
        }

        /* 2. Resource acquired (washer in use) */
        printf("    >>> [%s] GOT a washer! Washing for %dms at Tick: %u\n", pcTaskName, WASHING_TIME_MS, xTaskGetTickCount());
        
        /* Washing simulation */
        vTaskDelay(pdMS_TO_TICKS(WASHING_TIME_MS));

        /* 3. Release the semaphore (free the washer) */
        xSemaphoreGive(xCountingSemaphore);
        printf("    <<< [%s] RELEASED the washer at Tick: %u\n", pcTaskName, xTaskGetTickCount());

        /* Pause before the next washing attempt */
        vTaskDelay(pdMS_TO_TICKS(2000 + (rand() % 1000)));
    }
}

/* --- MAIN --- */
int main( void )
{
    prvInitialiseHeap();
    vTraceEnable(TRC_START);
    srand(xTaskGetTickCount()); // Initialize the random number generator

    printf("\nStarting Lab 4 (Variant 4: Counting Semaphore - %d Washers for %d Clients)...\n", MAX_WASHERS, NUM_CLIENTS);

    /* 1. Create the counting semaphore */
    /* xSemaphoreCreateCounting( Max_Count, Initial_Count ) */
    xCountingSemaphore = xSemaphoreCreateCounting(MAX_WASHERS, MAX_WASHERS);

    if (xCountingSemaphore != NULL) {
        /* 2. Create the client tasks */
        /* All clients have the same priority (1) */
        xTaskCreate(vClient, "Client1", 1024, NULL, 1, NULL);
        xTaskCreate(vClient, "Client2", 1024, NULL, 1, NULL);
        xTaskCreate(vClient, "Client3", 1024, NULL, 1, NULL);
        xTaskCreate(vClient, "Client4", 1024, NULL, 1, NULL);

        /* 3. Start the scheduler */
        vTaskStartScheduler();
    } else {
        printf("Failed to create counting semaphore.\n");
    }

    return 0;
}

/* --- STANDARD HOOKS --- */
void vApplicationMallocFailedHook( void ) { vAssertCalled( __LINE__, __FILE__ ); }
void vApplicationIdleHook( void ) { }
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char * pcTaskName ) { (void)pxTask; (void)pcTaskName; vAssertCalled( __LINE__, __FILE__ ); }
void vApplicationTickHook( void ) { }
void vApplicationDaemonTaskStartupHook( void ) { }

/* Allocations Helpers */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE * puxIdleTaskStackSize ) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize ) {
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
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

/* Trace Recorder Helpers */
static uint32_t ulEntryTime = 0;
void vTraceTimerReset( void ) { ulEntryTime = xTaskGetTickCount(); }
uint32_t uiTraceTimerGetFrequency( void ) { return configTICK_RATE_HZ; }
uint32_t uiTraceTimerGetValue( void ) { return( xTaskGetTickCount() - ulEntryTime ); }