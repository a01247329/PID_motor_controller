/*
 * prbs9.c
 *
 *  Created on: Nov 25, 2026
 *      Author: blanc
 */

#include "main.h"
#include "math.h"
#include "rpm_counter.h"
#include "uart_transmit.h"
#include "prbs9.h"

#define PRBS_LEN     511       // 2^9 - 1
#define T_BIT_MS     100       // duracion de cada bit (multiplo de 'period')
#define T_START_MS   1000      // inicio de la PRBS
#define T_END_MS     (T_START_MS + PRBS_LEN * T_BIT_MS)   // 52100 ms

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

void prbs9(void)
{
	if (current_time <= 1000)
	{
		duty_cycle = 0;
	}

	else if (current_time < 3000)
	{
		duty_cycle = 92;
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

}

