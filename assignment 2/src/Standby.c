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
#define TEMP_THRESHOLD 32
#define LIGHT_THRESHOLD 800
#define ACC_TOLERANCE 2

volatile uint32_t msTicks; // counter for 1ms SysTicks
int resetFlag;
int isSafe;
uint8_t gAccRead;

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

void pinsel_uart3(void){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
}

void init_uart(void){
	UART_CFG_Type uartCfg;
	uartCfg.Baud_rate = 115200;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;
	//pin select for uart3;
	pinsel_uart3();
	//supply power & setup working par.s for uart3
	UART_Init(LPC_UART3, &uartCfg);
	//enable transmit for uart3
	UART_TxCmd(LPC_UART3, ENABLE);
}

static uint8_t* acknowledged() {
	uint8_t data = 0;
	uint32_t len = 0;
	uint32_t haveRecieved;
	uint8_t line[64] = "";
	uint32_t currTime = msTicks;

	while ((msTicks - currTime < 5000) && (len<6) && (data != '\r')) {
		haveRecieved = UART_Receive(LPC_UART3, &data, 1, NONE_BLOCKING);
		if (haveRecieved != 0 && data != '\r') {
			line[len++] = data;
		}
	}
	return line;
}

void handshake() {
	char* recievedMsg;
	char* ready = "RDY 036 \r\n";
	char* established = "HSHK 036\r\n";
	while (1) {
		UART_Send(LPC_UART3, (uint8_t *)ready , strlen(ready), BLOCKING);
		recievedMsg = acknowledged();
		printf("%s\n", recievedMsg);
		if (!strcmp(recievedMsg,"RACK"))
			break;
	}
	UART_Send(LPC_UART3, (uint8_t *)established , strlen(ready), BLOCKING);
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
	init_uart();
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

static void quick_sort(int8_t arr[5],int low,int high) {
	int pivot,j,temp,i;
 	if(low<high) {
  		pivot = low;
  		i = low;
  		j = high;

  		while(i<j) {
   			while((arr[i]<=arr[pivot])&&(i<high))
    			i++;
 			while(arr[j]>arr[pivot])
    			j--;
		   	if(i<j) {
				temp=arr[i];
		    	arr[i]=arr[j];
		    	arr[j]=temp;
		   	}
  		}

  		temp=arr[pivot];
  		arr[pivot]=arr[j];
  		arr[j]=temp;
  		quick_sort(arr,low,j-1);
  		quick_sort(arr,j+1,high);
 	}
}

/* The function below actually falls below the Active.h but due to the systick timer, its here */
int calculateFreq(){
	int i, j;
	uint32_t runtime;
	int data[41];
	int finalData[37];
	int numOfReadings = 0;
	int numOfSamples = 0;
	int frequency = 0;
	int8_t x,y,z;
	int isMovingUp;
	int isInitialised = 0;
	uint32_t start_time = msTicks;
	while(1){
		runtime = msTicks - start_time;
		if(runtime > 1000) break;
		if(!(runtime%50)){ // get reading every 50ms
			acc_read(&x,&y,&z);
			data[numOfReadings++] = z;
		}
	}

	for(i=0;i<numOfReadings-4;i++){
		int8_t temp[5];

		for(j=0;j<5;j++)
			temp[j] = data[i+j];

		quick_sort(temp,0,4);
		finalData[numOfSamples++] = temp[2];
	}

	for(i=0;i<numOfSamples;i++){
		if(!isInitialised){
			if(finalData[i] > ACC_TOLERANCE + gAccRead){
				isInitialised = 1;
				isMovingUp = 1;
			}else if(finalData[i] < gAccRead - ACC_TOLERANCE){
				isInitialised = 1;
				isMovingUp = 0;
			}
		}else{
			if (finalData[i] > ACC_TOLERANCE + gAccRead && isMovingUp == 0)
				frequency++;
			else if (finalData[i] < gAccRead - ACC_TOLERANCE  && isMovingUp == 1)
				frequency++;
		}
	}
	uint32_t endTime = msTicks - start_time;
	printf("The calculation takes %d ms\n",endTime);
	return frequency;
}
