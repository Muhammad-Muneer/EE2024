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
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "led7seg.h"
#include "light.h"
#include "joystick.h"
#include "pca9532.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_uart.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "temp.h"
#include "light.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#define CONDITION_SAFE "Safe "
#define CONDITION_RISKY "Risky"


extern int resetFlag;
extern int isSafe;
extern uint8_t gAccRead;

void standbyInit();
void runTempAndLight(int* tempBool);
int calculateFreq();
void sendReadySignal()

#endif /* STANDBY_H_ */
/****************************************************************************
**                            End Of File
*****************************************************************************/
