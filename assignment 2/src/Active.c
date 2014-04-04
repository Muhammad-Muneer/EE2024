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

static void initBuzzer(void)
{
// Initialize button
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 31;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(1, 1<<31, 0);

	GPIO_SetDir(2, 1<<0, 1);
    GPIO_SetDir(2, 1<<1, 1);

    GPIO_SetDir(0, 1<<27, 1);
    GPIO_SetDir(0, 1<<28, 1);
    GPIO_SetDir(2, 1<<13, 1);
    GPIO_SetDir(0, 1<<26, 1);

    GPIO_ClearValue(0, 1<<27); //LM4811-clk
    GPIO_ClearValue(0, 1<<28); //LM4811-up/dn
    GPIO_ClearValue(2, 1<<13); //LM4811-shutdn
}

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);

static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;

    if (note > 0) {

        while (t < (durationMs*1000)) {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / 2);
            //delay32Us(0, note / 2);

            NOTE_PIN_LOW();
            Timer0_us_Wait(note / 2);
            //delay32Us(0, note / 2);

            t += note;
        }

    }
    else {
    	Timer0_Wait(durationMs);
        //delay32Ms(0, durationMs);
    }
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
	initBuzzer();
	MODE = ACTIVE_MODE;
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

	if(isFrequent >= 3)
		playNote(2272,1000);

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

