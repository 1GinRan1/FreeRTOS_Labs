/*
 * FreeRTOS Lab #2 - Variant 4
 */

 /* --- STANDARD INCLUDES --- */
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
#include "trcRecorder.h"

/* --- LAB 2 CONSTANTS (V-4) --- */
#define IDX             4   
#define N_TASKS         4   
#define BURST_TRIGGER   4   
#define TLS_CTX_INDEX   0   
#define TLS_PROF_INDEX  1   
#define BASE_DELAY_MS   130 
#define STEP_MS         7   
#define BURST_N         4   

/* --- HEAP CONFIGURATION --- */
#define mainREGION_1_SIZE     82010
#define mainREGION_2_SIZE     239050
#define mainREGION_3_SIZE     168070

/* --- FUNCTION PROTOTYPES --- */
void vAssertCalled(unsigned long ulLine, const char* const pcFileName);
static void prvInitialiseHeap(void);

/* Hooks prototypes */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName);
void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE* puxIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer, StackType_t** ppxTimerTaskStackBuffer, uint32_t* pulTimerTaskStackSize);
void vApplicationDaemonTaskStartupHook(void);

/* --- DATA STRUCTURES --- */
typedef enum run_mode_e {
    MODE_DELAY = 0,
    MODE_BUSY = 1
} run_mode_t;

/* Profile Structure  */
typedef struct profile_s {
    run_mode_t mode;
    uint8_t burstMode;
    uint32_t baseDelayMs;
    uint32_t baseCycles;
    uint8_t step;
} profile_t;

/* Task Context Structure */
typedef struct task_context_s {
    char name[12];
    uint32_t iter;
    uint32_t checksum;
    uint32_t delayTicksOrCycles;
    uint32_t seed;
    char line1[BURST_N][64]; /* Uses BURST_N */
    uint8_t bcnt;
} task_context_t;

/* --- HELPER FUNCTIONS --- */

/* CRC8 implementation as per the assignment */
uint8_t crc8_sae_j11850(uint8_t* data, size_t len) {
    uint8_t crc = 0xFF;
    size_t i;
    uint8_t j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x1D;
            }
            else {
                crc <<= 1;
            }
        }
    }
    return crc ^ 0xFF;
}

/* Flushing the log buffer */
void burst_flush(task_context_t* ctx) {
    int i;
    if (ctx->bcnt == 0) return;

    /* Critical section to prevent printf from interleaving */
    taskENTER_CRITICAL();
    for (i = 0; i < ctx->bcnt; i++) {
        printf("%s\n", ctx->line1[i]);
    }
    taskEXIT_CRITICAL();

    ctx->bcnt = 0;
}

/* --- WORKER TASK --- */
void vWorker(void* arg) {
    /* 1. Task initialization */
    int task_id = (int)(uintptr_t)arg;
    int Ni, i;
    uint8_t data_to_crc[8];
    TickType_t delay;

    /* Allocating memory for our "notebooks" */
    task_context_t* ctx = (task_context_t*)pvPortMalloc(sizeof(task_context_t));
    profile_t* prof = (profile_t*)pvPortMalloc(sizeof(profile_t));

    if (ctx == NULL || prof == NULL) {
        printf("Malloc failed for Task %d\n", task_id);
        if (ctx) vPortFree(ctx);
        if (prof) vPortFree(prof);
        vTaskDelete(NULL);
        return;
    }

    /* 2. Configure context */
    snprintf(ctx->name, sizeof(ctx->name), "T%02d_%d", IDX, task_id);
    ctx->iter = 0;
    ctx->checksum = 0;
    ctx->seed = 0xAA + task_id; /* Unique "seed" */
    ctx->bcnt = 0;

    /* 3. Configure profile */
    prof->mode = MODE_DELAY;
    prof->burstMode = BURST_TRIGGER; /* Burst enabled */
    prof->baseDelayMs = BASE_DELAY_MS;
    prof->step = STEP_MS;

    /* 4. Store pointers in "private pockets" (TLS) */
    vTaskSetThreadLocalStoragePointer(NULL, TLS_CTX_INDEX, ctx);
    vTaskSetThreadLocalStoragePointer(NULL, TLS_PROF_INDEX, prof);

    /* 5. Calculate loop parameters */
    Ni = 10 * N_TASKS + IDX; /* Ni = 10*4 + 4 = 44 */

    /* 6. Main work loop */
    for (i = 0; i < Ni; ++i) {
        ctx->iter++;

        /* Update checksum */
        memcpy(&data_to_crc[0], &ctx->iter, 4);
        memcpy(&data_to_crc[4], &ctx->seed, 4);
        ctx->checksum = crc8_sae_j11850(data_to_crc, 8);

        /* Write log to buffer */
        snprintf(ctx->line1[ctx->bcnt], 64,
            "[Task %s] Tick: %lu Iter: %lu Sum: 0x%08X",
            ctx->name, xTaskGetTickCount(), ctx->iter, ctx->checksum);

        ctx->bcnt++;

        /* If buffer is full - flush */
        if (ctx->bcnt >= prof->burstMode) {
            burst_flush(ctx);
        }

        /* Delay according to variant (delay) */
        delay = pdMS_TO_TICKS(prof->baseDelayMs + (prof->step * task_id));
        vTaskDelay(delay);
    }

    /* 7. Task completion */
    burst_flush(ctx); /* Flush remnants from the buffer */

    taskENTER_CRITICAL();
    printf("Task finished %s\n", ctx->name);
    taskEXIT_CRITICAL();

    /* Clean up memory */
    vPortFree(ctx);
    vPortFree(prof);
    vTaskDelete(NULL);
}

/* --- MAIN FUNCTION --- */
int main(void)
{
    int i;
    prvInitialiseHeap();
    vTraceEnable(TRC_START);

    printf("\nStarting Lab 2 (Variant 4)...\r\n");

    /* Create N_TASKS (4) tasks */
    for (i = 0; i < N_TASKS; ++i) {
        xTaskCreate(vWorker, "Task", 1024, (void*)(uintptr_t)i, 1, NULL);
    }

    vTaskStartScheduler();
    return 0;
}

/* --- HOOKS --- */
/* All hooks required for compilation are implemented here */
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

void vAssertCalled(unsigned long ulLine, const char* const pcFileName) {
    printf("ASSERT! Line %ld, file %s\r\n", ulLine, pcFileName);
    __debugbreak();
    for (;; );
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

/* ============================================================ */
/* TRACE RECORDER SUPPORT FUNCTIONS (FIX FOR LNK2019)        */
/* ============================================================ */
static uint32_t ulEntryTime = 0;

void vTraceTimerReset(void)
{
    ulEntryTime = xTaskGetTickCount();
}

uint32_t uiTraceTimerGetFrequency(void)
{
    return configTICK_RATE_HZ;
}

uint32_t uiTraceTimerGetValue(void)
{
    return(xTaskGetTickCount() - ulEntryTime);
}