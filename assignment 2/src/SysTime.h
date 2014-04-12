/*****************************************************************************
 *   Active.h:  Header file for System Time
 *   Author: Muhammad Muneer & Nicholas Chew
 *
******************************************************************************/
#ifndef SYSTIME_H_
#define SYSTIME_H

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "led7seg.h"
#include "light.h"
#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "temp.h"
#include "lpc17xx_uart.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

extern volatile uint32_t msTicks;
extern int UNSAFE_LOWER;
extern int UNSAFE_UPPER;
extern int TIME_WINDOW;
extern int REPORTING_TIME;
extern int isMayDay;

void SysTick_Handler();
uint32_t getSystick();
void delay(uint32_t delay);
void enableTime();

#endif /* end __ACTIVE_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/

