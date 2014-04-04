/*
 *  main.c
 *
 *  Created on: 22-Mar-2014
 *  Author: Muhammad Muneer & Nicholas Chew
 */

#include <string.h>
#include <stdio.h>
#include "lpc17xx_uart.h"
#include "Calibration.h"
#include "Standby.h"
#include "Active.h"

int resetFlag;
int isSafe;
uint8_t gAccRead;
static char* msg = NULL;

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

int main() {
	int isNormal;
	uint8_t data = 0;
	uint32_t len = 0;
	uint8_t line[64];
	init_uart();
	//test sending message
	msg = "Welcome to EE2024 \r\n";
	UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
	//test receiving a letter and sending back to port
	UART_Receive(LPC_UART3, &data, 1, BLOCKING);
	UART_Send(LPC_UART3, &data, 1, BLOCKING);
	//test receiving message without knowing message length
	len = 0;
	do
	{
		UART_Receive(LPC_UART3, &data, 1, BLOCKING);
			if (data != '\r')
			{
				len++;
				line[len-1] = data;
			}
	} while ((len<64) && (data != '\r'));
	line[len]=0;
	UART_SendString(LPC_UART3, &line);
	printf("--%s--\n", line);
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
						int freq = calculateFreq();
						printf("frequency: %d\n", freq);
						runActive(freq);
						runTempAndLight(&isNormal);
						//printf("Temp: %d, Rad: %d\n",isNormal,isSafe);
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

