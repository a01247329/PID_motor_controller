/*
 * uart_transmit.h
 *
 *  Created on: Nov 16, 2026
 *      Author: blanc
 */

#ifndef INC_UART_TRANSMIT_H_
#define INC_UART_TRANSMIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"

void uart_transmit(unsigned long current_time, int duty_cycle, unsigned long pulse_count);
void uart_end(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_UART_TRANSMIT_H_ */
