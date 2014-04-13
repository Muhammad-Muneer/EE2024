#include "Active.h"
#include "SysTime.h"

#define WARNING_MODE 1
#define ACTIVE_MODE 0
#define ACC_TOLERANCE 2
#define MAYDAY_THRESHOLD 5
#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);
#define MODE_NAME_X 1
#define MODE_NAME_Y 1

int isInAccRead;
int isFrequent;
static int MODE;
volatile uint32_t msTicks;
int UNSAFE_LOWER;
int UNSAFE_UPPER;
int TIME_WINDOW;
int REPORTING_TIME;
int reportingTimeFlag;
int isMayDay;

static void displayActive() {
	oled_putString(MODE_NAME_X,MODE_NAME_Y,(uint8_t*)"Active ",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

static void initBuzzer(void) {
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

static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;

    if (note > 0) {

        while (t < (durationMs*1000)) {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / 2);
            NOTE_PIN_LOW();
            Timer0_us_Wait(note / 2);
            t += note;
        }
    }
    else {
    	Timer0_Wait(durationMs);
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
	isMayDay = 0;
	enableAcc();
	disableSegment();
	displayActive();
	initBuzzer();
	MODE = ACTIVE_MODE;
	reportingTimeFlag = msTicks;
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

static void sendUARTAct(int freq) {
	if(msTicks - reportingTimeFlag > REPORTING_TIME * 1000){
		reportingTimeFlag = msTicks;
		if (MODE==WARNING_MODE) {
			char messageWarning[16];
			snprintf( messageWarning, sizeof(messageWarning), "013 %02d WARNING\r\n", freq);
			UART_Send(LPC_UART3, (uint8_t *)messageWarning ,strlen(messageWarning), BLOCKING);
		}
		else if (MODE==ACTIVE_MODE) {
			char messageActive[8];
			snprintf( messageActive, sizeof(messageActive), "036 %02d\r\n", freq);
			UART_Send(LPC_UART3, (uint8_t *)messageActive ,strlen(messageActive), BLOCKING);
		}
	}
}

void runActive(int freq){
	sendUARTAct(freq);
	if(!safe(freq)){
		isFrequent++;
	}else{
		isFrequent = 0;
	}
	if(isFrequent > MAYDAY_THRESHOLD) isMayDay = 1;

	if(isFrequent == TIME_WINDOW && MODE == ACTIVE_MODE){
		enterWarningMode();
	}else if(isFrequent < TIME_WINDOW && MODE == WARNING_MODE){
		leaveWarningMode();
	}

	if(isFrequent >= TIME_WINDOW)
		playNote(2272,1000);
}

void switchDisplayToStandby() {
	oled_putString(MODE_NAME_X,MODE_NAME_Y,(uint8_t*)"Standby",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(MODE==WARNING_MODE){
		leaveWarningMode();
	}
}

void switchDisplayToMayDay(){
	oled_clearScreen(OLED_COLOR_BLACK);
	uint8_t x_center = 96/2 - 7*6/2 ;
	uint8_t y_center = 64/2 - 3;
	oled_putString(x_center,y_center,(uint8_t*)"MAYDAY ",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(MODE==WARNING_MODE){
		leaveWarningMode();
	}
}

void switchDisplayToCalibrate(){
	if(MODE==WARNING_MODE){
		leaveWarningMode();
	}
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
			isInAccRead = 1;
			acc_read(&x,&y,&z);
			isInAccRead = 0;
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
				isMovingUp = 0;
			}else if(finalData[i] < gAccRead - ACC_TOLERANCE){
				isInitialised = 1;
				isMovingUp = 1;
			}
		}else{
			if ((finalData[i] > ACC_TOLERANCE + gAccRead && isMovingUp == 1) ||
				(finalData[i] < gAccRead - ACC_TOLERANCE  && isMovingUp == 0)){
				frequency++;
				isMovingUp = !isMovingUp;
			}
		}
	}
	return frequency;
}

void setVariables() {
	UNSAFE_LOWER = 2;
	UNSAFE_UPPER = 10;
	TIME_WINDOW = 3;
	REPORTING_TIME = 1;
}
