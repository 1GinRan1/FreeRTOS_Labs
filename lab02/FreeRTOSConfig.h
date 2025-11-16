#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* --- Basic Configuration --- */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      1
#define configTICK_RATE_HZ                      ( 1000 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 70 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 490 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 ( 12 )
#define configUSE_TRACE_FACILITY                1
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_RECURSIVE_MUTEXES             1
#define configQUEUE_REGISTRY_SIZE               20
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_APPLICATION_TASK_TAG          1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0
#define configUSE_QUEUE_SETS                    1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   5
#define configSUPPORT_STATIC_ALLOCATION         1
#define configINITIAL_TICK_COUNT                ( ( TickType_t ) 0 )
#define configSTREAM_BUFFER_TRIGGER_LEVEL_TEST_MARGIN 1

/* --- LAB 2 CRITICAL SETTING: TLS POINTERS --- */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5 

#define configUSE_TIME_SLICING                  1

/* --- Architecture Specific --- */
#ifdef _M_X64
    #define configTICK_TYPE_WIDTH_IN_BITS       TICK_TYPE_WIDTH_64_BITS
#else
    #define configTICK_TYPE_WIDTH_IN_BITS       TICK_TYPE_WIDTH_32_BITS
#endif

/* --- Timers --- */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                20
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )
#define configMAX_PRIORITIES                    ( 7 )

/* --- Runtime Stats --- */
#define configRUN_TIME_COUNTER_TYPE             uint64_t
configRUN_TIME_COUNTER_TYPE ulGetRunTimeCounterValue( void );
void vConfigureTimerForRunTimeStats( void );
#define configGENERATE_RUN_TIME_STATS           1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() ulGetRunTimeCounterValue()

/* --- Co-routines --- */
#define configUSE_CO_ROUTINES                   1
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* --- Formatting --- */
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* --- API Inclusion --- */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xSemaphoreGetMutexHolder        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1

/* --- Testing --- */
#define configRUN_ADDITIONAL_TESTS              1

/* --- Assert --- */
extern void vAssertCalled( unsigned long ulLine, const char * const pcFileName );
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __LINE__, __FILE__ )

/* --- Windows Specific --- */
#ifdef WIN32_LEAN_AND_MEAN
    #include "winsock2.h"
#else
    #include <winsock.h>
#endif

/* --- Trace --- */
#include "trcRecorder.h"

#endif /* FREERTOS_CONFIG_H */
