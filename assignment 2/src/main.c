/*
 *  main.c
 *
 *  Created on: 22-Mar-2014
 *  Author: Muhammad Muneer & Nicholas Chew
 */

#include <string.h>
#include <stdio.h>
#include "Calibration.h"
#include "Standby.h"
#include "Active.h"

int resetFlag;
int isSafe;

int main() {
	uint8_t gAccRead;
	int isNormal;
	while (1) {
		calibrateInit();
		displayCalibrate();
		if(isCalibrated(&gAccRead)){
			standbyInit();
			while(1){
				if (resetFlag) break;
				runTempAndLight(&isNormal);
				if (isSafe && isNormal){
					initActive();
					while(1){
						//initLight();
						//initActive();
						runTempAndLight(&isNormal);
						printf("Temp: %d, Rad: %d\n",isNormal,isSafe);
						runActive();
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

/*****************
 * 	Pseudo code
 * int main() {
	calibrateInit();
	while (1) {
		displayCalibrate();
		if standby button is pressed {
			while 1 {
				if reset is pressed break
				initStandby
				standbyRun(&temp,&light)
				if( temp && light is within safety) {
					while 1 {
						if reset is pressed break
						goes to active mode
					}
					break;
				}
			}
		}
	}
}
 * */
