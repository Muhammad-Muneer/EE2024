/*****************************************************************************
 *   Calibration.h:  Header file for Calibration mode
 *
 *   Copyright(C) 2009, Embedded Artists AB
 *   All rights reserved.
 *
******************************************************************************/
#ifndef __CALIBRATION_H
#define __CALIBRATION_H

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "led7seg.h"
#include "lpc17xx_nvic.h"
#include "rgb.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include <string.h>

void calibrateInit(void);
void displayCalibrate();
uint8_t isCalibrated(uint8_t* accReading);
uint8_t isResetted();

#endif /* end __CALIBRATE_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
