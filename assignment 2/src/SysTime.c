/*
 * Systemtime.c
 *
 *  Created on: 12-Apr-2014
 *      Author: AdminNUS
 */

#include "SysTime.h"

volatile uint32_t msTicks;

void SysTick_Handler(void) {
  	msTicks++;
}

uint32_t getSystick(void){
	return msTicks;
}

void delay(uint32_t delay) {
	uint32_t currentTime = msTicks;
	while (msTicks - currentTime < delay);
}

void enableTime(){
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1);  // Capture error
	}
}
