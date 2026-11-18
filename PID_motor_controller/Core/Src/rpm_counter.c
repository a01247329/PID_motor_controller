/*
 * rpm_counter.c
 *
 *  Created on: Nov 15, 2026
 *      Author: blanc
 */

#include <rpm_counter.h>
#include "main.h"
#include "math.h"
#include "uart_transmit.h"

//ESCALON
/*#define K_MODEL   7.024f
#define TAU_MODEL 0.0300f*/

//PRBS9
#define PRBS_LEN     511       // 2^9 - 1
#define T_BIT_MS     100       // duracion de cada bit (multiplo de 'period')
#define T_START_MS   1000      // inicio de la PRBS
#define T_END_MS     (T_START_MS + PRBS_LEN * T_BIT_MS)   // 52100 ms

uint32_t start = 0;
uint32_t period = 10;
uint32_t current_time = 0;
float mtr_rpm_real = 0;
float mtr_rpm_modelo = 0;
static uint8_t finished = 0;

//PRBS9
static uint16_t lfsr = 0x1FF;  // semilla distinta de cero
static uint32_t bit_count = 0;
static uint8_t  level = 0;

static uint8_t prbs9_next(void)
{
	uint8_t out = lfsr & 1;
	uint16_t fb = ((lfsr >> 8) ^ (lfsr >> 4)) & 1;
	lfsr = ((lfsr << 1) | fb) & 0x1FF;
	return out;
}

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

	if (current_time <= 1000)
	{
		duty_cycle = 0;
		//ESCALON
		//mtr_rpm_modelo = 0;
	}

	else if (current_time < 3000)
	{
		duty_cycle = 92;
		//ESCALON
		/*float te = (current_time - 1000) / 1000.0f;
		mtr_rpm_modelo = K_MODEL * duty_cycle * (1.0f - expf(-te / TAU_MODEL));*/
		//PRBS9
		uint32_t k = (current_time - T_START_MS) / T_BIT_MS;

		while (bit_count <= k)
		{
			level = prbs9_next();
			bit_count++;
		}

		duty_cycle = level ? duty_cycle : 0;

	}

	else
	{
		duty_cycle = 0;
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
		uart_end();
		finished = 1;
		return;
	}

	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ARR * duty_cycle / 100);

}

