/*
 * Standby.c
 *
 * Author: Muhammad Muneer & Nicholas Chew
 */

#include "Standby.h"

#define CONDITION_NORMAL "Normal"
#define CONDITION_HOT "Hot   "
#define CONDITION_SAFE "Safe "
#define CONDITION_RISKY "Risky"
#define TEMP_THRESHOLD 20
#define LIGHT_THRESHOLD 800

volatile uint32_t msTicks; // counter for 1ms SysTicks
int resetFlag;
int isSafe;

static void initEINT0Interupt() {
	PINSEL_CFG_Type PinCfg;

	/* Initialize GPIO pin connect */
	PinCfg.Funcnum = 1;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PINSEL_ConfigPin(&PinCfg);
}

void EINT0_IRQHandler () {
	//printf("EINT0 IS CALLED\n");
	if ((LPC_SC -> EXTINT) & 0x01) {
		//printf("In EINT0: The reset button is pressed\n");
		resetFlag = 1;
		LPC_SC -> EXTINT = (1<<0);
	}
}

void  EINT3_IRQHandler() {
	if ((LPC_GPIOINT -> IO2IntStatF>>5) & 0x01) {
		NVIC_DisableIRQ(EINT3_IRQn);
		isSafe = !isSafe;
		if (isSafe) {
			light_setLoThreshold(0);
			light_setHiThreshold(800);
		}
		else {
			light_setHiThreshold(3891);
			light_setLoThreshold(800);
		}
		LPC_GPIOINT -> IO2IntClr = 1<<5;
		light_clearIrqStatus();
		NVIC_EnableIRQ(EINT3_IRQn);
	}
}

static void enableResetBtn() {
	LPC_SC -> EXTINT = (1<<0);
	initEINT0Interupt();
	NVIC_EnableIRQ(EINT0_IRQn);
}

static void disableCalibratorBtn() {
	GPIO_ClearValue(1, 1<<31);
}

static void displayModeName() {
	char modeName[] = "Standby";
	uint8_t i = 1;
	uint8_t j = 1;
	oled_putString(i,j,modeName,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void displayStandby() {
	oled_clearScreen(OLED_COLOR_BLACK);
	displayModeName();
	oled_putString(50,30,CONDITION_SAFE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

//  SysTick_Handler - just increment SysTick counter
void SysTick_Handler(void) {
  msTicks++;
}

uint32_t getSystick(void){
	return msTicks;
}

static void initTemp() {
	PINSEL_CFG_Type PinCfg;

	/* Initialize GPIO pin connect */
	PinCfg.Funcnum = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(0, 1<<2, 0);
	SysTick_Config(SystemCoreClock / 1000);
	temp_init(&getSystick);
}

static void delay(uint32_t delay) {

	uint32_t currentTime = msTicks;
	while (msTicks - currentTime < delay);
}

static void countDown() {
	char i;
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1);  // Capture error
	}
	for (i='5'; i>='0';i--){
		led7seg_setChar(i,FALSE);
		if (resetFlag)
			break;
		delay(1000);
	}
}

void initLight() {
	LPC_GPIOINT -> IO2IntClr = 1<<5;
	light_clearIrqStatus();
	light_enable();
	light_setRange(LIGHT_RANGE_4000);
	light_setWidth(LIGHT_WIDTH_16BITS);
	LPC_GPIOINT -> IO2IntEnF |= 1<<5;
	light_setHiThreshold(800);
	light_setLoThreshold(0);
	//light_setIrqInCycles(2);
	NVIC_EnableIRQ(EINT3_IRQn);
}

static void disableAcc() {
	acc_setMode(0x00);
}

void standbyInit(){
	resetFlag = 0;
	isSafe = 1;
	//printf("Inside standbyInit. Value is set to 1. IsSafe: %d\n",isSafe);
	disableAcc();
	displayStandby();
	enableResetBtn();
	disableCalibratorBtn();
	countDown();
	initTemp();
	initLight();
	oled_putString(50,30,CONDITION_SAFE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void displayTemp(int32_t temp,int isNormal){
	uint8_t i = 30;
	uint8_t j = 15;
	int counter = 0;
	while(temp > 0){
		i -= 6;
		if(counter !=1){
			char reading = '0';
			int last_d = temp%10;
			reading += last_d;
			oled_putChar(i,j,reading,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
			temp /=10;
		}else{
			oled_putChar(i,j,'.',OLED_COLOR_WHITE,OLED_COLOR_BLACK);
		}
		counter++;
	}

	if(isNormal)
		oled_putString(50,j,CONDITION_NORMAL,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	else
		oled_putString(50,j,CONDITION_HOT,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(isSafe)
		oled_putString(50,30,CONDITION_SAFE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	else
		oled_putString(50,30,CONDITION_RISKY,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

void runTempAndLight(int* tempBool) {
	int32_t tempRead = temp_read();
	*tempBool = (tempRead/10.0 < TEMP_THRESHOLD);
	displayTemp(tempRead,*tempBool);
}


