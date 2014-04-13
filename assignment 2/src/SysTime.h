/*****************************************************************************
 *   Active.h:  Header file for System Time
 *   Author: Muhammad Muneer & Nicholas Chew
 *
******************************************************************************/
#ifndef SYSTIME_H_
#define SYSTIME_H

#include "lpc17xx_timer.h"

extern volatile uint32_t msTicks;
extern int UNSAFE_LOWER;
extern int UNSAFE_UPPER;
extern int TIME_WINDOW;
extern int REPORTING_TIME;
extern int isMayDay;
extern int distressResponse;
extern int isInAccRead;
extern int standbyFlag;

void SysTick_Handler();
uint32_t getSystick();
void delay(uint32_t delay);
void enableTime();

#endif /* end __ACTIVE_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/

