/*
 * uart_transmit.c
 *
 *  Created on: Nov 16, 2026
 *      Author: blanc
 */

#include <stdio.h>
#include "usart.h"
#include "main.h"
#include "uart_transmit.h"

void uart_transmit(unsigned long current_time, int duty_cycle, unsigned long pulse_count)
{
	char buf[40];
	int n = snprintf(buf, sizeof(buf), "%lu,%d,%lu\r\n",
	                 (unsigned long)current_time, duty_cycle, (unsigned long)pulse_count);
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, n, 10);
}

void uart_end()
{
	HAL_UART_Transmit(&huart2, (uint8_t*)"END\r\n", 5, 10);
}

