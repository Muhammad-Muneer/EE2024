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

int resetFlag;
int isSafe;
uint8_t gAccRead;

int main() {
	int isNormal;
	while (1) {
		calibrateInit();
		displayCalibrate();
		if(isCalibrated(&gAccRead)){
			standbyInit();
			while(1){
				if (resetFlag) break;
				sendReadySignal();
				runTempAndLight(&isNormal);
				if (isSafe && isNormal){
					initActive();
					while(1){
						int freq = calculateFreq();
						printf("frequency: %d\n", freq);
						runActive(freq);
						runTempAndLight(&isNormal);
						//printf("Temp: %d, Rad: %d\n",isNormal,isSafe);
						if (resetFlag)
							break;
						if(!isNormal || !isSafe){
							switchDisplayToStandby();
							break;
						}
					}
					if(resetFlag){
						switchDisplayToCalibrate();
						break;
					}
				}
			}
		}
	}
}


