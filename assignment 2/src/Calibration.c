/*
 * Calibration.c
 *
 *  Created on: 22-Mar-2014
 *  Author: Muhammad Muneer, Nicholas Chew
 *
 */

#include "Calibration.h"
#include <stdio.h>

static void initI2C() {
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void initAccelerometer(){
	initI2C();
	acc_init();
}

static void initSSP() {
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);
}

static void initOled() {
	oled_init();
}

static void displayMode() {
	char modeName[] = "Calibration";
	uint8_t i = 1;
	uint8_t j = 1;
	oled_putString(i,j,modeName,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void displayGravitationalAcc() {
	int8_t x,y,z;
	int isNegative = 0;
	acc_read(&x,&y,&z);
	printf("%d\n",z- 80);
	char str[10] = "Z: ";
	uint8_t i = 1;
	uint8_t j = 15;
	oled_putString(i,j,str,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	i += 30;
	if(z==0){
		oled_putChar(i,j,'0',OLED_COLOR_WHITE,OLED_COLOR_BLACK);
		return;
	}

	if(z<0){
		isNegative = 1;
		z *= -1;
	}
	while(z > 0){
		i -= 6;
		char reading = '0';
		int last_d = z%10;
		reading += last_d;
		oled_putChar(i,j,reading,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
		z /=10;
	}

	if(isNegative){
		i -= 6;
		oled_putChar(i,j,'-',OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	}

}

void displayCalibrate() {
	oled_clearScreen(OLED_COLOR_BLACK);
	displayMode();
	displayGravitationalAcc();
	int i=0;
	for (i=0; i <10000000; i++);
}

static void disableResetBtn() {
	NVIC_DisableIRQ(EINT3_IRQn);
}

static void enableCalibrateBtn() {
	PINSEL_CFG_Type PinCfg;

	/* Initialize GPIO pin connect for SW4 */
	PinCfg.Funcnum = 0;
	PinCfg.Pinnum = 31;
	PinCfg.Portnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(1, 1<<31, 0);
}

static void disable7Segment() {
	initSSP();
	led7seg_init();
	led7seg_setChar('~',FALSE);
}

static void disableRGB() {
	PINSEL_CFG_Type PinCfg;

	/* Initialize GPIO pin connect */
	PinCfg.Funcnum = 0;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PINSEL_ConfigPin(&PinCfg);

	rgb_init();
	rgb_setLeds(0);
}

static void disableLEDS(){
	pca9532_init();
	uint16_t ledOff = 0x0000;
	pca9532_setLeds(ledOff,0xffff);
}

void calibrateInit(void) {
	disable7Segment();
	disableRGB();
	disableResetBtn();
	enableCalibrateBtn();
	initAccelerometer();
//	disableLEDS();
	initOled();
}

uint8_t isCalibrated(int8_t* accReading){
	int8_t x,y,z;
	acc_read(&x,&y,&z);
	uint8_t input = (GPIO_ReadValue(1)>>31)&0x01;
	*accReading = z;
	return !input;
}

uint8_t isResetted(){
	uint8_t input = (GPIO_ReadValue(2)>>10)&0x01;
	return !input;
}

