#include "MayDay.h"
#include "SysTime.h"

#define NO 0
#define YES 1
#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);
int distressResponse;
int planeState[96][64];
int input;
char noOfSuccAttempt;

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

void initMayDay(){
	distressResponse = 0;
	light_shutdown();
}

static void sendAndReceiveSignal(){
	int right_light = 0x00ff;
		int left_light = (0x00ff<<8);
		while(1){
			if(distressResponse)  {
				pca9532_setLeds(0x0000,0xffff);
				break;
			}
			UART_Send(LPC_UART3, (uint8_t *)"MAY DAY!! Obstacles Ahead\r\n",strlen("MAY DAY!! Obstacles Ahead\r\n"), BLOCKING);
			playNote(2551,300);
			pca9532_setLeds(right_light|left_light,0xffff);
			right_light = (right_light>>1);
			left_light = (left_light<<1);
			if(right_light == 0){
				right_light = 0x00ff;
				left_light = (0x00ff<<8);
			}
			playNote(3030,300);
			// break out of the loop when signal is sent
		}
}

static void initJoystick(){
	joystick_init();
}

static void prepareOled(){
	oled_clearScreen(OLED_COLOR_BLACK);
	oled_putString(96/2-33,0,(uint8_t *)"NEED HELP ?",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(96/2-9,20,(uint8_t *)"YES",OLED_COLOR_BLACK,OLED_COLOR_WHITE);
	oled_putString(96/2-6,30,(uint8_t *)"NO",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	input = YES; //default = YES
}

static int getInputFromUser(){
	uint8_t joyState;
	while(1){
		joyState = joystick_read();
		if((joyState & JOYSTICK_UP) != 0 && input == NO){
			oled_putString(96/2-9,20,(uint8_t *)"YES",OLED_COLOR_BLACK,OLED_COLOR_WHITE);
			oled_putString(96/2-6,30,(uint8_t *)"NO",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
			input = YES;
		}else if((joyState & JOYSTICK_DOWN) != 0 && input == YES){
			oled_putString(96/2-9,20,(uint8_t *)"YES",OLED_COLOR_WHITE,OLED_COLOR_BLACK);
			oled_putString(96/2-6,30,(uint8_t *)"NO",OLED_COLOR_BLACK,OLED_COLOR_WHITE);
			input = NO;
		}else if((joyState & JOYSTICK_CENTER) != 0){
			oled_clearScreen(OLED_COLOR_BLACK);
			break;
		}
	}

	return input;
}

static int respondBackToSignal(){
	int input;
	initJoystick();
	prepareOled();
	input = getInputFromUser();
	return input;
}

static void drawBody(int index){

	int i,j;
	oled_fillRect(2,index+6,20,index+8,OLED_COLOR_WHITE);
	for(i=2;i<=20;i++){
		for(j=index+6;j<=index+8;j++){
			planeState[i][j] = 1;
		}
	}
}

static void oled_placePixel(uint8_t x, uint8_t y, oled_color_t color){
	oled_putPixel(x,y,color);
	planeState[x][y] = 1;
}

static void drawLeftUpperWing(int index){
	oled_placePixel(2,index+3,OLED_COLOR_WHITE);
	oled_placePixel(2,index+4,OLED_COLOR_WHITE);
	oled_placePixel(2,index+5,OLED_COLOR_WHITE);
	oled_placePixel(3,index+4,OLED_COLOR_WHITE);
	oled_placePixel(3,index+5,OLED_COLOR_WHITE);
	oled_placePixel(4,index+5,OLED_COLOR_WHITE);
}

static void drawLeftBtmWing(int index){
	oled_placePixel(2,index+9,OLED_COLOR_WHITE);
	oled_placePixel(2,index+10,OLED_COLOR_WHITE);
	oled_placePixel(2,index+11,OLED_COLOR_WHITE);
	oled_placePixel(3,index+9,OLED_COLOR_WHITE);
	oled_placePixel(3,index+10,OLED_COLOR_WHITE);
	oled_placePixel(4,index+9,OLED_COLOR_WHITE);
}

static void drawRightUpperWing(int index){
	oled_placePixel(9,index,OLED_COLOR_WHITE);
	oled_placePixel(9,index+1,OLED_COLOR_WHITE);
	oled_placePixel(10,index+1,OLED_COLOR_WHITE);
	oled_placePixel(10,index+2,OLED_COLOR_WHITE);
	oled_placePixel(11,index+2,OLED_COLOR_WHITE);
	oled_placePixel(10,index+3,OLED_COLOR_WHITE);
	oled_placePixel(11,index+3,OLED_COLOR_WHITE);
	oled_placePixel(12,index+3,OLED_COLOR_WHITE);
	oled_placePixel(11,index+4,OLED_COLOR_WHITE);
	oled_placePixel(12,index+4,OLED_COLOR_WHITE);
	oled_placePixel(13,index+4,OLED_COLOR_WHITE);
	oled_placePixel(14,index+4,OLED_COLOR_WHITE);
	oled_placePixel(12,index+5,OLED_COLOR_WHITE);
	oled_placePixel(13,index+5,OLED_COLOR_WHITE);
	oled_placePixel(14,index+5,OLED_COLOR_WHITE);
	oled_placePixel(15,index+5,OLED_COLOR_WHITE);
}

static void drawRightBtmWing(int index){
	oled_placePixel(9,index+14,OLED_COLOR_WHITE);
	oled_placePixel(9,index+13,OLED_COLOR_WHITE);
	oled_placePixel(10,index+13,OLED_COLOR_WHITE);
	oled_placePixel(10,index+12,OLED_COLOR_WHITE);
	oled_placePixel(11,index+12,OLED_COLOR_WHITE);
	oled_placePixel(10,index+11,OLED_COLOR_WHITE);
	oled_placePixel(11,index+11,OLED_COLOR_WHITE);
	oled_placePixel(12,index+11,OLED_COLOR_WHITE);
	oled_placePixel(11,index+10,OLED_COLOR_WHITE);
	oled_placePixel(12,index+10,OLED_COLOR_WHITE);
	oled_placePixel(13,index+10,OLED_COLOR_WHITE);
	oled_placePixel(14,index+10,OLED_COLOR_WHITE);
	oled_placePixel(12,index+9,OLED_COLOR_WHITE);
	oled_placePixel(13,index+9,OLED_COLOR_WHITE);
	oled_placePixel(14,index+9,OLED_COLOR_WHITE);
	oled_placePixel(15,index+9,OLED_COLOR_WHITE);
}

static void drawWings(int index){
	drawLeftUpperWing(index);
	drawLeftBtmWing(index);
	drawRightUpperWing(index);
	drawRightBtmWing(index);
}

static void drawNoseAndTail(int index){
	oled_placePixel(0,index+7,OLED_COLOR_WHITE);
	oled_placePixel(1,index+7,OLED_COLOR_WHITE);
	oled_placePixel(21,index+7,OLED_COLOR_WHITE);
}

static void drawPlane(int index){
	int i,j;
	for(i=0;i<96;i++)
		for(j=0;j<64;j++)
			planeState[i][j] = 0;

	drawBody(index);
	drawWings(index);
	drawNoseAndTail(index);
}

static void drawOutline(){
	oled_line(0,0,95,0,OLED_COLOR_WHITE);
	oled_line(0,1,95,1,OLED_COLOR_WHITE);
	oled_line(0,63,95,63,OLED_COLOR_WHITE);
	oled_line(0,62,95,62,OLED_COLOR_WHITE);
}

static void drawGameSprites(int index){
	drawPlane(index);
	drawOutline();
}

static void moveObs(int obs[3][2],int index,int* obsAdded){
	int i;
	int j;
	int flag;
	oled_clearScreen(OLED_COLOR_BLACK);
	drawGameSprites(index);
	for(i=0;i<3;i++){
		flag = 0;
		obs[i][0]--;
		for(j=0;j<6 && !flag;j++){
			if(obs[i][0]+j > 0 && obs[i][0]+j < 95){
				oled_line(obs[i][0]+j,0,obs[i][0]+j,obs[i][1],OLED_COLOR_WHITE);
				oled_line(obs[i][0]+j,obs[i][1]+20,obs[i][0]+j,63,OLED_COLOR_WHITE);
			}
			else{
				flag = 1;
			}
		}
	}
}

static void updateObs(int obs[3][2],int canAdd,int* obsAdded){
	if(canAdd){
		obs[(*obsAdded)%3][0] = 95;
		obs[(*obsAdded)%3][1] = (rand() % 37) + 8;
		(*obsAdded) = (*obsAdded) + 1;
		if((*obsAdded) > 2)
			noOfSuccAttempt++;
	}

}

static int checkCollision(int x, int y){
	int j,k;
	int isColliding = 0;
	for(j=x;j<x+6 && j<96 && j>0;j++){
		for(k=0;k<=y && k<64;k++){
			if(planeState[j][k] == 1)
				return !isColliding;
		}
		for(k=y+20;k<64;k++){
			if(planeState[j][k] == 1)
				return !isColliding;
		}
	}
	return isColliding;
}

static void enterGameMode(){
	uint8_t joyState;
	noOfSuccAttempt = '0';
	led7seg_init();
	int obsAdded = 0;
	int isColliding = 0;
	int counter = 0;
	int obsPos[3][2] = {{0}};
	int index = 64/2 - 7;
	drawGameSprites(index);
	led7seg_setChar(noOfSuccAttempt,FALSE);
	while(1){
		joyState = joystick_read();
		if(obsAdded > 0){
			moveObs(obsPos,index,&obsAdded);
			isColliding = checkCollision(obsPos[obsAdded%3][0],obsPos[obsAdded%3][1]);
		}
		if((joyState & JOYSTICK_UP) != 0){
			index-=2;
			oled_clearScreen(OLED_COLOR_BLACK);
			drawGameSprites(index);
			isColliding  = checkCollision(obsPos[obsAdded%3][0],obsPos[obsAdded%3][1]);
		}else if((joyState & JOYSTICK_DOWN) != 0){
			index+=2;
			oled_clearScreen(OLED_COLOR_BLACK);
			drawGameSprites(index);
			isColliding = checkCollision(obsPos[obsAdded%3][0],obsPos[obsAdded%3][1]);
		}
		if(isColliding){
			playNote(3030,100);
			isColliding = 0;
		}
		updateObs(obsPos,!(counter%45),&obsAdded);
		led7seg_setChar(noOfSuccAttempt,FALSE);
		if(noOfSuccAttempt == '9'){
			break;
		}
		counter++;
		if(index < 2 || index+13 > 61){
			playNote(3030,100);
		}
	}
}

void runMayDay(){
	int response;
	sendAndReceiveSignal();
	response = respondBackToSignal();
	if(!response){
		enterGameMode();
		led7seg_setChar('~',TRUE);
		oled_clearScreen(OLED_COLOR_BLACK);
	}
}


