/*
 * escalon.c
 *
 *  Created on: Nov 25, 2026
 *      Author: blanc
 */

#include "main.h"
#include "math.h"
#include "rpm_counter.h"
#include "uart_transmit.h"
#include "escalon.h"

#define K_MODEL   3.075f
#define TAU_MODEL 0.0300f

float mtr_rpm_real = 0;
float mtr_rpm_modelo = 0;

void paso_escalon(void)
{
	if (current_time <= 1000)
	{
		duty_cycle = 0;
		mtr_rpm_modelo = 0;
	}

	else if (current_time < 3000)
	{
		duty_cycle = 92;
		float te = (current_time - 1000) / 1000.0f;
		mtr_rpm_modelo = K_MODEL * duty_cycle * (1.0f - expf(-te / TAU_MODEL));
	}

	else
	{
		duty_cycle = 0;
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
		uart_end();
		finished = 1;
		return;
	}

}
