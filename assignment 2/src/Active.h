/*****************************************************************************
 *   Active.h:  Header file for Active mode
 *   Author: Muhammad Muneer & Nicholas Chew
 *
******************************************************************************/
#ifndef __ACTIVE_H
#define __ACTIVE_H

#include "lpc17xx_pinsel.h"
#include "pca9532.h"
#include "oled.h"
#include "rgb.h"
#include <string.h>
#include "lpc17xx_gpio.h"
#include "led7seg.h"
#include "acc.h"
#include "lpc17xx_uart.h"

extern uint8_t gAccRead; 
extern int isFrequent;
void initActive();
void runActive(int freq);
void switchDisplayToStandby();
void switchDisplayToCalibrate();
void switchDisplayToMayDay();
int calculateFreq();
void resetTimeForActSig();
void sendActiveSignal();
void setVariables();

#endif /* end __ACTIVE_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
