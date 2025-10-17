#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void Th0(void * pvParameters);
void Th1(void * pvParameters);
void Th2(void * pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
SemaphoreHandle_t xSemaphoreIns;
SemaphoreHandle_t xSemaphoreOuts;
//SemaphoreHandle_t xSemaphoreBin;	// Uso otro semáforo para evitar que la sección del conteo se ejcute todo el tiempo
TaskHandle_t xHandleTh2 = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_SW2_IRQ_HANDLER(void)	// Salidas
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    /* Clear external interrupt flag. */
    GPIO_GpioClearInterruptFlags(BOARD_SW2_GPIO, 1U << BOARD_SW2_GPIO_PIN);
#else
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW2_GPIO, 1U << BOARD_SW2_GPIO_PIN);
#endif
    /* Change state of button. */
    xSemaphoreGiveFromISR( xSemaphoreOuts, &xHigherPriorityTaskWoken );
    SDK_ISR_EXIT_BARRIER;
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void BOARD_SW3_IRQ_HANDLER(void)	// Entradas
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    /* Clear external interrupt flag. */
    GPIO_GpioClearInterruptFlags(BOARD_SW3_GPIO, 1U << BOARD_SW3_GPIO_PIN);
#else
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW3_GPIO, 1U << BOARD_SW3_GPIO_PIN);
#endif
    /* Change state of button. */
    xSemaphoreGiveFromISR( xSemaphoreIns, &xHigherPriorityTaskWoken );
    SDK_ISR_EXIT_BARRIER;
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

int main(void) {
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    // Habilitar interrupciones de los botones
    NVIC_SetPriority(BOARD_SW2_IRQ, 10);
    NVIC_SetPriority(BOARD_SW3_IRQ, 10);
    EnableIRQ(BOARD_SW2_IRQ);
    EnableIRQ(BOARD_SW3_IRQ);

    xSemaphoreIns = xSemaphoreCreateCounting( 10, 0 );	// Semáforo de entradas
    xSemaphoreOuts = xSemaphoreCreateCounting( 10, 0 ); // Semáforo de salidas

    xTaskCreate(Th0, "Thread 0", 512, NULL, 1, NULL);	// Control de entradas
	xTaskCreate(Th1, "Thread 1", 512, NULL, 1, NULL);	// Control de salidas
	xTaskCreate(Th2, "Thread 2", 512, NULL, 2, &xHandleTh2);	// Estado del estacionamiento

	vTaskStartScheduler();
}

void Th0(void * pvParameters){
//	static uint8_t entradas = 0;
	while(1){
		xSemaphoreTake(xSemaphoreIns, portMAX_DELAY);
		PRINTF("Ingresa auto \r\n");

		/* Notificar a Th2: bit 0 = entrada */
		if (xHandleTh2 != NULL)
		{
			xTaskNotify(xHandleTh2, (1 << 0), eSetBits);
		}
	}
}

void Th1(void * pvParameters){
//	static uint8_t salidas = 0;
	while(1){
		xSemaphoreTake(xSemaphoreOuts, portMAX_DELAY);
		PRINTF("Sale auto \r\n");

		/* Notificar a Th2: bit 1 = salida */
		if (xHandleTh2 != NULL)
		{
			xTaskNotify(xHandleTh2, (1 << 1), eSetBits);
		}
	}
}

void Th2(void * pvParameters){
	uint8_t cupo = 10;
	uint32_t eventos = 0;

	PRINTF("Iniciando control de estacionamiento\r\n");
	while(1){
		/* Esperar notificación de Th0 o Th1 */
		xTaskNotifyWait(0, 0xFFFFFFFF, &eventos, portMAX_DELAY);

		/* Revisar qué evento llegó */
		if (eventos & (1 << 0)) // Entrada
		{
			if (cupo > 0) cupo--;
		}
		if (eventos & (1 << 1)) // Salida
		{
			if (cupo < 10) cupo++;
		}

		/* Mostrar estado actual */
		PRINTF("Cupo disponible: %d\r\n", cupo);

		        /* Control de LEDs */
		if(cupo <=10 && cupo >= 7){			// Luz verde
			GPIO_PortClear(BOARD_LED_GREEN_GPIO, 1u << BOARD_LED_GREEN_GPIO_PIN);
			GPIO_PortSet(BOARD_LED_RED_GPIO, 1u << BOARD_LED_RED_GPIO_PIN);
		}
		else if(cupo <=6 && cupo >= 1){		// Luz amarilla
			GPIO_PortClear(BOARD_LED_RED_GPIO, 1u << BOARD_LED_RED_GPIO_PIN);
			GPIO_PortClear(BOARD_LED_GREEN_GPIO, 1u << BOARD_LED_GREEN_GPIO_PIN);
		}
		else if(cupo == 0){					// Luz roja
			GPIO_PortClear(BOARD_LED_RED_GPIO, 1u << BOARD_LED_RED_GPIO_PIN);
			GPIO_PortSet(BOARD_LED_GREEN_GPIO, 1u << BOARD_LED_GREEN_GPIO_PIN);
		}
	}
}
