/*
 * rpm_counter.c
 *
 *  Created on: Nov 15, 2026
 *      Author: blanc
 */

#include "rpm_counter.h"
#include "main.h"
#include "math.h"
#include "uart_transmit.h"
#include "escalon.h"
#include "prbs9.h"
#include "pid.h"

uint8_t prueba = 2;
uint32_t start = 0;
uint32_t period = 10;

void rpm_counter(void)
{
	current_time = HAL_GetTick();

	if (finished)
	{
		static uint32_t last_end = 0;
		if ((current_time - last_end) >= 50)
		{
			last_end = current_time;
			uart_end();
		}
		return;
	}

	if ((current_time - start) >= period)
	{
		start = current_time;
		__disable_irq();
		uint32_t pc = pulse_count;
		pulse_count = 0;
		__enable_irq();
		mtr_rpm_real = (pc * 60000.0f / period) / 85.0f;
		uart_transmit(current_time, duty_cycle, pc);
	}

	if (prueba == 1)
	{
		paso_escalon();
		//pid();
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ARR * duty_cycle / 100);
	}

	else if (prueba == 2)
	{
		prbs9();
		//pid();
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ARR * duty_cycle / 100);
	}

}

