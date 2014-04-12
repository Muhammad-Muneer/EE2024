/*
 *  main.c
 *
 *  Created on: 22-Mar-2014
 *  Author: Muhammad Muneer & Nicholas Chew
 */

#include <string.h>
#include <stdio.h>
#include "lpc17xx_uart.h"
#include "Calibration.h"
#include "Standby.h"
#include "Active.h"
#include "MayDay.h"

int resetFlag;
int standbyFlag;
int isSafe;
uint8_t gAccRead;
int hasEstablished;
int isMayDay;

int main() {
	int isNormal;
	setVariables();
	while (1) {
		calibrateInit();
		displayCalibrate();
		if(isCalibrated(&gAccRead)){
			standbyInit();
			while(1){
				if (resetFlag) break;
				sendReadySignal();
				runTemp(&isNormal);
				if (isSafe && isNormal  && hasEstablished){
					initActive();
					while(1){
						int freq = calculateFreq();
						runActive(freq);
						runTemp(&isNormal);
						if (resetFlag)
							break;

						if(isMayDay){
							switchDisplayToMayDay();
							break;
						}

						if(!isNormal || !isSafe || standbyFlag){
							standbyFlag = 0;
							switchDisplayToStandby();
							break;
						}
					}
					if(resetFlag){
						switchDisplayToCalibrate();
						break;
					}

					if(isMayDay){
						initMayDay();
						while(1){
							//inMayDay
						}
					}
				}
			}
		}
	}
}


