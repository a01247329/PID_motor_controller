/*
 * pwm_generator.c
 *
 *  Created on: Nov 11, 2026
 *      Author: blanc
 */
#include <pwm_generator.h>
#include "main.h"

void pwm_generator(void)
{

	for (duty_cycle = 0; duty_cycle < 100; duty_cycle++)
	{
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ARR * duty_cycle / 100);
		HAL_Delay(10);
	}

	for (duty_cycle = 100; duty_cycle > 0; duty_cycle--)
	{
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ARR * duty_cycle / 100);
		HAL_Delay(10);
	}
}
