/*****************************************************************************
 *   Standby.h:  Header file for Standby mode
 *   Author: Muhammad Muneer & Nicholas Chew
 *
******************************************************************************/

#ifndef STANDBY_H_
#define STANDBY_H_

#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "led7seg.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_uart.h"
#include "acc.h"
#include "oled.h"
#include "temp.h"
#include "light.h"
#include <string.h>
#include <ctype.h>

extern int resetFlag;
extern int isSafe;
extern uint8_t gAccRead;

void standbyInit();
void runTemp(int* tempBool);
void sendReadySignal();
void countDown();

#endif /* STANDBY_H_ */
/****************************************************************************
**                            End Of File
*****************************************************************************/
