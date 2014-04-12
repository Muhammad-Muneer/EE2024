/*
 * Standby.c
 *
 * Author: Muhammad Muneer & Nicholas Chew
 */

#include "Standby.h"
#include "SysTime.h"

#define CONDITION_NORMAL "Normal"
#define CONDITION_HOT "Hot   "
#define CONDITION_SAFE "Safe "
#define CONDITION_RISKY "Risky"
#define TEMP_THRESHOLD 30
#define LIGHT_THRESHOLD 800

volatile uint32_t msTicks;
int resetFlag;
int standbyFlag;
int isSafe;
int hasEstablished;
uint32_t timeForRdySig;
uint8_t bufferForUART[5];
int buffer_counter;
uint8_t gAccRead;
int UNSAFE_LOWER;
int UNSAFE_UPPER;
int TIME_WINDOW;
int REPORTING_TIME;

static void initEINT0Interupt(){
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 1;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PINSEL_ConfigPin(&PinCfg);
}

void EINT0_IRQHandler () {
	if ((LPC_SC -> EXTINT) & 0x01) {
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

void pinsel_uart3(void){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
}

static void clearBuffer(){
	int i;
	for(i=0;i<5;i++)
		bufferForUART[i] = '\0';
}

void UART_INTERUPT(){
	uint32_t haveRecieved;
	uint8_t data = 0;
	haveRecieved = UART_Receive(LPC_UART3, &data, 1, NONE_BLOCKING);
	if (haveRecieved != 0 && data != '\r') {
		if (buffer_counter < 5)
			bufferForUART[buffer_counter] = data;
		else
			return;
		buffer_counter++;
	}

	// checking for commands below

	if(data == '\r'){
		timeForRdySig = msTicks;
		if (buffer_counter > 5){
			buffer_counter = 0;
			clearBuffer();
			return;
		}

		if(!strcmp(bufferForUART,"RNACK"))
			UART_Send(LPC_UART3, (uint8_t *)"RDY 036 \r\n" , strlen("RDY 036 \r\n"), BLOCKING);
		else if (!strcmp(bufferForUART,"RACK")){
			UART_Send(LPC_UART3, (uint8_t *)"HSHK 036\r\n" ,strlen("HSHK 036\r\n"), BLOCKING);
			hasEstablished = 1;
		} else if (!strcmp(bufferForUART,"DEBUG")){
			UART_Send(LPC_UART3, (uint8_t *)"NOT ESTABLISHED\r\n" ,strlen("NOT ESTABLISHED\r\n"), BLOCKING);
			hasEstablished = 0;
		} else if (!strcmp(bufferForUART,"RSTC")) {
			UART_Send(LPC_UART3, (uint8_t *)"CACK\r\n" ,strlen("CACK\r\n"), BLOCKING);
			resetFlag = 1;
		}
		else if (!strcmp(bufferForUART,"RSTS")) {
			UART_Send(LPC_UART3, (uint8_t *)"SACK\r\n" ,strlen("SACK\r\n"), BLOCKING);
			standbyFlag = 1;
		}else if (strstr(bufferForUART,"UL ") != NULL ) {
			if(isdigit(bufferForUART[3])){
				int num  = (bufferForUART[3] - '0');
				if(isdigit(bufferForUART[4])) num = num*10 + (bufferForUART[4] - '0');
				if(num <= 0 || num >= UNSAFE_UPPER){
					UART_Send(LPC_UART3, (uint8_t *)"YOU MAD BRO\r\n",strlen("YOU MAD BRO\r\n"), BLOCKING);
				}else {
					UNSAFE_LOWER = num;
					char messageSuccess[25];
					snprintf( messageSuccess, sizeof(messageSuccess), "UNSAFE LOWER IS NOW %d\r\n", UNSAFE_LOWER);
					UART_Send(LPC_UART3, (uint8_t *)messageSuccess ,strlen(messageSuccess), BLOCKING);
				}
			} else {
				UART_Send(LPC_UART3, (uint8_t *)"INVALID LA BRO\r\n" ,strlen("INVALID LA BRO\r\n" ), BLOCKING);
			}
		}else if (strstr(bufferForUART,"UU ") != NULL ) {
			if(isdigit(bufferForUART[3])){
				int num  = (bufferForUART[3] - '0');
				if(isdigit(bufferForUART[4])) num = num*10 + (bufferForUART[4] - '0');
				if(num <= UNSAFE_LOWER || num >= 20){
					UART_Send(LPC_UART3, (uint8_t *)"YOU MAD BRO\r\n",strlen("YOU MAD BRO\r\n"), BLOCKING);
				}else {
					UNSAFE_UPPER = num;
					char messageSuccess[25];
					snprintf( messageSuccess, sizeof(messageSuccess), "UNSAFE UPPER IS NOW %d\r\n", UNSAFE_UPPER);
					UART_Send(LPC_UART3, (uint8_t *)messageSuccess ,strlen(messageSuccess), BLOCKING);
				}
			} else {
				UART_Send(LPC_UART3, (uint8_t *)"INVALID LA BRO\r\n" ,strlen("INVALID LA BRO\r\n" ), BLOCKING);
			}
		}else if (strstr(bufferForUART,"TW ") != NULL ) {
			if(isdigit(bufferForUART[3])){
				int num  = (bufferForUART[3] - '0');
				if(isdigit(bufferForUART[4])) num = num*10 + (bufferForUART[4] - '0');
				if(num <= 0 || num >= 5){
					UART_Send(LPC_UART3, (uint8_t *)"YOU MAD BRO\r\n",strlen("YOU MAD BRO\r\n"), BLOCKING);
				}else {
					TIME_WINDOW = num;
					char messageSuccess[25];
					snprintf( messageSuccess, sizeof(messageSuccess), "TIME WINDOW IS NOW %d\r\n", TIME_WINDOW);
					UART_Send(LPC_UART3, (uint8_t *)messageSuccess ,strlen(messageSuccess), BLOCKING);
				}
			} else {
				UART_Send(LPC_UART3, (uint8_t *)"INVALID LA BRO\r\n" ,strlen("INVALID LA BRO\r\n" ), BLOCKING);
			}
		}else if (strstr(bufferForUART,"RT ") != NULL ) {
			if(isdigit(bufferForUART[3])){
				int num  = (bufferForUART[3] - '0');
				if(isdigit(bufferForUART[4])) num = num*10 + (bufferForUART[4] - '0');
				if(num <= 0 || num >= 10){
					UART_Send(LPC_UART3, (uint8_t *)"YOU MAD BRO\r\n",strlen("YOU MAD BRO\r\n"), BLOCKING);
				}else {
					REPORTING_TIME = num;
					char messageSuccess[25];
					snprintf( messageSuccess, sizeof(messageSuccess), "REPORTING TIME IS NOW %d\r\n", REPORTING_TIME);
					UART_Send(LPC_UART3, (uint8_t *)messageSuccess ,strlen(messageSuccess), BLOCKING);
				}
			} else {
				UART_Send(LPC_UART3, (uint8_t *)"INVALID LA BRO\r\n" ,strlen("INVALID LA BRO\r\n" ), BLOCKING);
			}
		}
		else {
			UART_Send(LPC_UART3, (uint8_t *)"INVALID LA BRO\r\n" ,strlen("INVALID LA BRO\r\n"), BLOCKING);
		}
		buffer_counter = 0;
		clearBuffer();
	}
}

void UART3_IRQHandler(void) {
	UART3_StdIntHandler();
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

	// Below is for enabling UART interrupt
	NVIC_EnableIRQ(UART3_IRQn);
	UART_IntConfig(LPC_UART3, UART_INTCFG_RBR, ENABLE);
	UART_SetupCbs(LPC_UART3, 0, UART_INTERUPT);
}

void sendReadySignal(){
	if(!hasEstablished && msTicks - timeForRdySig > 5000){
		UART_Send(LPC_UART3, (uint8_t *)"RDY 036 \r\n" , strlen("RDY 036 \r\n"), BLOCKING);
		timeForRdySig = msTicks;
	}
}

static void countDown() {
	char i;
	timeForRdySig = msTicks;
	for (i='5'; i>='0';i--){
		sendReadySignal();
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
	NVIC_EnableIRQ(EINT3_IRQn);
}

static void disableAcc() {
	acc_setMode(0x00);
}

void standbyInit(){
	standbyFlag = 0;
	resetFlag = 0;
	isSafe = 1;
	hasEstablished = 0;
	buffer_counter = 0;
	clearBuffer();
	enableTime();
	disableAcc();
	displayStandby();
	enableResetBtn();
	disableCalibratorBtn();
	init_uart();
	initTemp();
	initLight();
	countDown();
	oled_putString(50,30,CONDITION_SAFE,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void displayTemp(int32_t temp,int isNormal) {
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

void runTemp(int* tempBool) {
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
