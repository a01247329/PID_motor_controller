/*
 * pid.c
 *
 *  Created on: Nov 25, 2026
 *      Author: blanc
 */

#include "main.h"
#include "pid.h"

float error = 0.0;
float error_prev = 0.0;
float set_point = 322.0;
float kp = 0.0;
float ki = 0.0;
float kd = 0.0;
float Ts = 5;

float proporcional;
float integral = 0.0;
float integral_prev;
float derivativo = 0.0;

void pid(void)
{
	error = set_point - mtr_rpm_real;
	integral = integral_prev + (error * Ts);
	derivativo = (error - error_prev) / Ts;
	duty_cycle = (kp * error) + (ki * integral) + (kd * derivativo);
	integral_prev = integral;
	error_prev = error;
}
