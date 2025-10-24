/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ONE_SHOT_TIMER_ID 0
#define AUTO_RELOAD_TIMER_ID 1

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 2)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);
static void vCallbackFunctionExample( TimerHandle_t xTimer );

TimerHandle_t OneShotSt;
TimerHandle_t AutoReloadSt;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    if (xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }

    OneShotSt = xTimerCreate("One-Shot Timer", pdMS_TO_TICKS(5000), pdFALSE, (void*)ONE_SHOT_TIMER_ID, vCallbackFunctionExample);
    AutoReloadSt = xTimerCreate("AutoReload Timer", pdMS_TO_TICKS(500), pdTRUE, (void*)AUTO_RELOAD_TIMER_ID, vCallbackFunctionExample);

    xTimerStart(OneShotSt, 0);
    xTimerStart(AutoReloadSt, 0);

    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void hello_task(void *pvParameters)
{
    for (;;)
    {
        PRINTF("Hello world.\r\n");
        vTaskSuspend(NULL);
    }
}

static void vCallbackFunctionExample( TimerHandle_t xTimer ){
	uint32_t TimerID;

	TimerID = (uint32_t)pvTimerGetTimerID(xTimer);

	if(TimerID == ONE_SHOT_TIMER_ID){
		PRINTF("OneShot Timer expired at %i ticks\r\n", xTaskGetTickCount());
	}else if(TimerID == AUTO_RELOAD_TIMER_ID){
		PRINTF("AutoReload Timer expired at %i ticks\r\n", xTaskGetTickCount());
	}
}
