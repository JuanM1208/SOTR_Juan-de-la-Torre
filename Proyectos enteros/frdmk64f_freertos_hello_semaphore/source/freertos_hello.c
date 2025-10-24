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
#include "semphr.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
SemaphoreHandle_t MySemaphore;


/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);
void Th0(void *pvParameters);
void Th1(void *pvParameters);

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

    MySemaphore = xSemaphoreCreateBinary();

    if (xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    if (xTaskCreate(Th0, "Thread 0", configMINIMAL_STACK_SIZE + 100, NULL, 3, NULL) !=
		pdPASS)
	{
		PRINTF("Task creation failed!.\r\n");
		while (1)
			;
	}
    if (xTaskCreate(Th1, "Thread 1", configMINIMAL_STACK_SIZE + 100, NULL, 2, NULL) !=
		pdPASS)
	{
		PRINTF("Task creation failed!.\r\n");
		while (1)
			;
	}

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

void Th0(void *pvParameters){
	while(1){
		xSemaphoreTake(MySemaphore, portMAX_DELAY);
		PRINTF("Processing data from Thread 1\n\r");
	}
}
void Th1(void *pvParameters){
	while(1){
		for (uint32_t i = 0; i < 5000; i++);
		PRINTF("Data processed from Thread 0\n\r");
		xSemaphoreGive(MySemaphore);
	}
}
