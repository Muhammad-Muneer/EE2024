/*****************************************************************************
 *   Active.h:  Header file for Active mode
 *   Author: Muhammad Muneer & Nicholas Chew
 *
******************************************************************************/
#ifndef __ACTIVE_H
#define __ACTIVE_H

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "led7seg.h"
#include "light.h"
#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "temp.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

void initActive();
void runActive();
void switchDisplayToStandby();
void switchDisplayToCalibrate();

#endif /* end __ACTIVE_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
