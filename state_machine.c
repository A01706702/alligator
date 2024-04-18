/*
 * state_machine.c
 *
 * Created: 16/04/2024
 *  Author: JPMB
 */ 

#ifndef F_CPU
#define F_CPU 9216000UL
#endif

#include <string.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include "allinone.h"
#include "lcdi2c.h"
#include "DrvUSART.h"
#include "DrvSYS.h"
#include "state_machine.h"

State currentState = IDLE; //start state
extern char *MESSAGE;
extern char INBUFF[0x20];
extern bool data_received;
bool commandOK = false;
bool commandERROR = false; //flags to check if command was succesful or not

void initStateMachine(void) {
	currentState = RTC;
}

void computeStateMachine(void) {
	switch (currentState){
		case IDLE: {
			//lcdSendStr("idle");
			if (!(PIND & (1 << 2))) { // PD2->1 (wait for button pressed)
				MESSAGE = "ATI\r\n"; //change message (debug)
				currentState = READING_DATA;
			}
			//currentState = RTC;
			break;
		}
		case RTC:{
			//lcdSendStr("rtc");
			//time real vs time actual
			//if overflow: READING DATA, else: IDLE
			currentState = READING_DATA;
		}
		case READING_DATA: {
			//lcdSendStr("rd: ");
			//readTemperature();
			//char str[20];
			//sprintf(str,"%f",readTemperature());
			//lcdSendStr(str);
			//readLight();
			sendATCommands(MESSAGE);
			
			// ask for GPS coords from BG95 //
			//char *MESSAGES []= {"AT\r\n", "ATI\r\n", "att\r\n"};
			//sendATCommands(MESSAGES[0]);
			//if (commandOK){
				//commandOK = false;
				//lcdSendStr("1!");
				//sendATCommands(MESSAGES[1]);
				//if (commandOK) {
					//commandOK = false;
					//lcdSendStr("2!");
					//sendATCommands(MESSAGES[2]);
					//if(commandOK){
						//clear();
						//lcdSendStr("wtf!");
					//}
					//else{
						//clear();
						//lcdSendStr("else");
						//if (commandERROR) {
							//clear();
							//lcdSendStr("yeah");
							//new_show_BUFF();
						//}
					//}
				//}
			//}
			//int i = 0;
			//while (i<3){
				//char str[20];
				//sprintf(str,"%d",i);
				//lcdSendStr(str);
				//sendATCommands(MESSAGES[i]);
				//if(commandOK){
					//i++;
				//}
				//if (commandERROR){
					//new_show_BUFF();
				//}
			//}
			//char str[20];
			//sprintf(str, "%d",commandOK);
			//lcdSendStr(str);
			
			//int i = 0;
			//sendATCommands(MESSAGES[i]);
			//
			//if(commandOK){i++;}
			//sendATCommands(MESSAGES[i]);
			//
			//if(commandOK){i++;}
			//sendATCommands(MESSAGES[i]);
			//new_show_BUFF();
			
			//sendATCommands("ATI\r\n"); //OK
			//sendATCommands("AT+QGPS=1\r\n"); //OK
			//sendATCommands("AT+QGPSLOC?"); //GPS COORDS
			//char COORDS[0x60] = {0};
			//for (int i = 0; i < sizeof(INBUFF)/sizeof(INBUFF[0]); i++) {
			//COORDS[i] = INBUFF[i];
			//}
			// GNSS off: //
			//sendATCommands("AT+QGPSEND "); //OK
			
			// if coming from idle: IDLE, else: SENDING DATA
			currentState = SENDING_DATA;
			break;
		}
		case SENDING_DATA: {
			//lcdSendStr("sd");
			//publish data to cloud probably
			currentState = IDLE;
			break;
		}
		default:
		break;
	}
}

void sendATCommands(char *msg) {
	//DrvUSART_SendStr("at\r\n"); //debug quitar
	DrvUSART_SendStr(msg);
	
	//read_UART(); //debug
	new_read_UART(); // works perfect
	new_show_BUFF(); //works for both cases
	
	//clear();
	//lcdSendStr(INBUFF); //raw print of the INBUFF (useful)
}

/* WORKS PERFECT! PRINTS EVERY OUTPUT*/
void new_read_UART() {
	commandOK = false;
	commandERROR = false;
	char search[] = "OK\r\n";
	char search2[] = "ERROR";	
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		INBUFF[i] = caracter;
		i++;
		if (INBUFF[i-1] == 0x0a) {
			char *ok = strstr(INBUFF, search);
			char *error = strstr(INBUFF, search2);
			if (ok) {
				commandOK = true;
				commandERROR = false;
				break;
			}
			if (error) {
				commandOK = false;
				commandERROR = true;
				break;
			}
		}		
	}
	INBUFF[i] = '\0'; //null terminate
}

/* WORKS PERFECT! */
void new_show_BUFF(){
	int i = 0;
	char caracter=0x00;
	clear();
	while(1) {
		caracter = INBUFF[i];
		i++;
		if (caracter==0x0a) {
			lcdSendStr("lf"); //send space
		}
		else {
			if (caracter==0x0d) {
				lcdSendStr("cr");
			}
			else {
				if (caracter == '\0') {
					break;
				}
				lcdSendChar(caracter);
			}
		}
		// iterate until null
		if (caracter == '\0') {
			break;
		}
	}
}

float readTemperature() {
	float temp = 32.5; //read temperature data from a sensor
	return temp;
}

void readLight() {
	//read light data from a sensor
}

void controlLED(bool state) {
	if (state) {
		PORTB |= (1 << 0); // Turn on PB0 LED
	}
	else {
		PORTB &= ~(1 << 0); // Turn off PB0 LED
	}
}

/* Original UART - NOT STORING LF AND CR */
/*
void read_UART() {
	char search[] = "OK";
	char search2[] = "ERROR";
	//char search3[] = "ERROR "; //when the error has a code we need to store
	
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		if (caracter == 0x0A) { // Line feed
			// Do nothing, skip storing it
		}
		else if (caracter == 0x0D) { // Carriage return
			// Do nothing, skip storing it
		}
		else {
			INBUFF[i] = caracter;
			i++;
		}
		// iterate until finding "OK or ERROR"
		char *ok = strstr(INBUFF, search);
		char *error = strstr(INBUFF, search2);
		if (ok || error) {
			break;
		}
	}
	INBUFF[i] = '\0'; //null terminate
}

// WORKS! (original 2) - STORING LF AND CR
void read_UART() {
	char search[] = "OK\r\n";
	char search2[] = "ERROR\r\n";
	//char search3[] = "ERROR "; //when the error has a code we need to store
	
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		INBUFF[i] = caracter;
		i++;
		// iterate until finding "OK or ERROR"
		char *ok = strstr(INBUFF, search);
		char *error = strstr(INBUFF, search2);
		if (ok || error) {
			break;
		}
	}
	INBUFF[i] = '\0'; //null terminate
}

*/
