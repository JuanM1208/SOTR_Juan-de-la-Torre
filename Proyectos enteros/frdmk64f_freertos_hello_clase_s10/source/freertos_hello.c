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

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 3)
#define hello_task2_PRIORITY (configMAX_PRIORITIES - 4)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);
static void hello_task2(void *pvParameters);

TaskHandle_t Task2;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t Calc(uint32_t a){
    if(a<=1){
        return a;
    }else{
        return(Calc(a-1) + Calc(a-2));
    }
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ){
	if(xTask == Task2){
		PRINTF("Task2 stack overflow\r\n");
		vTaskSuspend(Task2);
	}
}
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

    if (xTaskCreate(hello_task2, "Hello_task2", configMINIMAL_STACK_SIZE + 100, NULL, hello_task2_PRIORITY, &Task2) !=
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
        PRINTF("Estoy vivo :D!!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



static void hello_task2(void *pvParameters)
{
    uint32_t counter = 0;
    uint32_t val = 0;
    uint32_t *ptr = &val;

    for (;;)
    {
        for(counter=0;counter<40;counter++){
//        	if(uxTaskGetStackHighWaterMark(NULL)>8){
        		*(ptr-counter) = Calc(2+counter);
				PRINTF("Val: %i\n\r",*(ptr-counter));
//        	}
//        	else{
//        		PRINTF("Task 2 stack overflow warning\r\n");
//        		vTaskSuspend(NULL);
//        	}
        }
        vTaskSuspend(NULL);
    }
}
