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

typedef struct{
	uint8_t thread_ID;
	uint16_t data;
}Data_t;

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)

#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof( Data_t )
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);
static void Tx0_task(void *pvParameters);
static void Tx1_task(void *pvParameters);
static void Tx2_task(void *pvParameters);
static void Rx_task(void *pvParameters);

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

    QueueHandle_t xQueue;

	/* Create the queue, storing the returned handle in the xQueue variable. */
	xQueue = xQueueCreate( QUEUE_LENGTH, QUEUE_ITEM_SIZE );
	if( xQueue == NULL )
	{
		PRINTF("Queue creation failed!\r\n");
	}

    if (xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(Tx0_task, "Tx0_task", configMINIMAL_STACK_SIZE + 100,  ( void * ) xQueue, 0, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(Tx1_task, "Tx1_task", configMINIMAL_STACK_SIZE + 100,  ( void * ) xQueue, 0, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(Tx2_task, "Tx2_task", configMINIMAL_STACK_SIZE + 100,  ( void * ) xQueue, 0, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(Rx_task, "Rx_task", configMINIMAL_STACK_SIZE + 100,  ( void * ) xQueue, 1, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
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

static void Tx0_task(void *pvParameters){
	QueueHandle_t xQueue;
	Data_t xData;
	xData.thread_ID = 0;
	xData.data = 0;

	xQueue = ( QueueHandle_t ) pvParameters;
	for (;;)
	{
		if( xQueueSendToBack( xQueue, &xData, 10 ) != pdPASS )
		{
			PRINTF("Queue Full!\r\n");
		}
		else if(xData.data <= 65535){
			xData.data++;
		}
		else{
			xData.data = 0;
		}
	}
}

static void Tx1_task(void *pvParameters){
	QueueHandle_t xQueue;
	Data_t xData;
	xData.thread_ID = 1;
	xData.data = 65535;

	xQueue = ( QueueHandle_t ) pvParameters;
	for (;;)
	{
		if( xQueueSendToBack( xQueue, &xData, 10 ) != pdPASS )
		{
			PRINTF("Queue Full!\r\n");
		}
		else if(xData.data >= 0){
			xData.data--;
		}
		else{
			xData.data = 65535;
		}
	}
}

static void Tx2_task(void *pvParameters){
	QueueHandle_t xQueue;
	Data_t xData;
	xData.thread_ID = 2;
	xData.data = 0;

	xQueue = ( QueueHandle_t ) pvParameters;
	for (;;)
	{
		if(xData.data % 2 == 0){
			if( xQueueSendToBack( xQueue, &xData, 10 ) != pdPASS )
			{
				PRINTF("Queue Full!\r\n");
			}
		}
		if(xData.data <= 65535){
			xData.data++;
		}
		else{
			xData.data = 0;
		}
	}
}

static void Rx_task(void *pvParameters){
	QueueHandle_t xQueue;
	Data_t xData;

	xQueue = ( QueueHandle_t ) pvParameters;
	for (;;)
	{
		if( xQueueReceive( xQueue, &xData, portMAX_DELAY ) != pdPASS )
		{
			PRINTF("Queue Empty!\r\n");
		}
		else
		{
			PRINTF("Datos recibidos del Th%d = %d\r\n", xData.thread_ID, xData.data);
		}
	}
}
