/*
 * Active.c
 *
 *  Created on: 23-Mar-2014
 *      Author: AdminNUS
 */

#include "Active.h"
#define WARNING_MODE 1
#define ACTIVE_MODE 0
#define UNSAFE_LOWER 2
#define UNSAFE_UPPER 10
#define CONDITION_SAFE "Safe "
#define CONDITION_ACTIVE "Active"

int isFrequent;
static int MODE;

static void displayModeAct() {
	char modeName[] = "Active ";
	uint8_t i = 1;
	uint8_t j = 1;
	oled_putString(i,j,modeName,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(50,30,CONDITION_SAFE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void displayActive() {
	displayModeAct();
	oled_putString(50,30,CONDITION_ACTIVE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void enableAcc(){
	acc_setMode(0x01);
}

static void disableSegment(){
	led7seg_setChar('~',FALSE);
}

void initActive() {
	isFrequent = 0;
	enableAcc();
	disableSegment();
	displayActive();
	MODE = ACTIVE_MODE;
	//LPC_GPIOINT -> IO2IntClr = 1<<5;
	//light_clearIrqStatus();
}

static int safe(int freq){
	return (freq < UNSAFE_LOWER) || (freq > UNSAFE_UPPER);
}

static void enableLeds() {
	pca9532_init();
	uint16_t ledOn = 0xffff;
	pca9532_setLeds(ledOn,ledOn);
}

static void enableRedLed() {
	GPIO_SetDir( 2, (1<<0) , 1 );
	GPIO_SetValue( 2, (1<<0));
}

static void enterWarningMode(){
	MODE = WARNING_MODE;
	enableRedLed();
	enableLeds();
}

static void disableLeds(){
	uint16_t ledOff = 0x0000;
	pca9532_setLeds(ledOff,0xffff);
}

static void disableRedLed() {
	GPIO_ClearValue( 2, (1<<0) );
}

void leaveWarningMode(){
	MODE = ACTIVE_MODE;
	disableLeds();
	disableRedLed();
}


void runActive(int freq){
	if(!safe(freq)){
		isFrequent++;
	}else{
		isFrequent = 0;
	}

	if(isFrequent == 3 && MODE == ACTIVE_MODE){
		enterWarningMode();
	}else if(isFrequent < 3 && MODE == WARNING_MODE){
		leaveWarningMode();
	}
}

void switchDisplayToStandby() {
	char modeName[] = "Standby";
	uint8_t i = 1;
	uint8_t j = 1;
	oled_putString(i,j,modeName,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(50,30,CONDITION_SAFE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(MODE==WARNING_MODE){
		leaveWarningMode();
	}
}

void switchDisplayToCalibrate(){
	if(MODE==WARNING_MODE){
		leaveWarningMode();
	}
}

