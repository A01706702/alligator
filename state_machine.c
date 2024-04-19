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
extern char INBUFF[0x60];
extern char TEMP[0x78];
extern char COORDS[0x60];
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
			/* SOME TESTS */
			/*
			//char *MESSAGES []= {"ati\r\n", "at\r\n", "att\r\n"}; //caso de prueba falla no mover
			//char *MESSAGES []= {"at\r\n", "ati\r\n", "att\r\n"}; //sirve
			clear();
			lcdSendStr("Buffers:");
			_delay_ms(2000);
			
			MESSAGE = "at\r\n";
			test_sendATCommands(MESSAGE, TEMP, sizeof(TEMP)/sizeof(TEMP[0]));
			clear();
			lcdSendStr(TEMP);
			_delay_ms(2000);
						
			MESSAGE = "at\r\n";
			test_sendATCommands(MESSAGE, TEMP, sizeof(TEMP)/sizeof(TEMP[0]));
			clear();
			lcdSendStr(TEMP);
			_delay_ms(2000);
			
			clear();
			lcdSendStr("Done");
			*/
			
			
			sendATCommands("AT+QGPS=1\r\n"); //OK or ERROR (means its already on)
			_delay_ms(2000);
			test_sendATCommands("AT+QGPSLOC?\r\n", COORDS, sizeof(COORDS)/sizeof(COORDS[0])); //GPS COORDS
			_delay_ms(4000);
			/*
			//char COORDS[0x60] = {0};
			//for (int i = 0; i < sizeof(INBUFF)/sizeof(INBUFF[0]); i++) {
				//COORDS[i] = INBUFF[i];
			//}
			*/
			// GNSS off: //
			//test_sendATCommands("AT+QGPSEND\r\n", TEMP, sizeof(TEMP)/sizeof(TEMP[0]));
			test_sendATCommands("ATT\r\n", TEMP, sizeof(TEMP)/sizeof(TEMP[0]));
			_delay_ms(2000);
			
			clear();
			lcdSendStr("finished");
			
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
	DrvUSART_SendStr(msg);
	new_read_UART(); // works perfect
	new_show_BUFF(); //works for both cases
}

void test_sendATCommands(char *msg, char *buffer, size_t buff_size) {
	DrvUSART_SendStr(msg);
	test_read_UART(buffer, buff_size);
	//test_show_BUFF(buffer);
}

void clear_Buffer(char *buffer){ 
	memset(buffer, 0, sizeof(buffer)); //fill with int 0 (equivalent to char null)	
	//for (int j = 0; j < sizeof(buffer)/sizeof(buffer[0]); j++) {
		//buffer[j] = '\0';
	//}
}

/* WORKS PERFECT! PRINTS EVERY OUTPUT*/
//try using a temporary buffer
//try using pointer to OK
/*
void new_read_UART() {
	clear_Buffer(INBUFF);
	//commandOK = false;
	//commandERROR = false;
	char search[] = "OK\r\n";
	char search2[] = "ERROR";	
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		INBUFF[i] = caracter;
		i++;
		if (caracter == 0x0a) {
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
*/

/* ALSO WORKS PERFECT! */
void new_read_UART() {
	clear_Buffer(INBUFF);
	char search[] = "OK\r\n";
	char search2[] = "ERROR";
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		INBUFF[i] = caracter;
		i++;
		
		if (i >= sizeof(INBUFF) - 1) {
			break;
		}
		
		if (caracter == '\n') {
			char *ok = strstr(INBUFF, search);
			char *error = strstr(INBUFF, search2);
			if (ok || error) {
				commandOK = ok != NULL;
				commandERROR = error != NULL;
				break;
			}
		}
	}
	INBUFF[i] = '\0'; // Null terminate the string
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
			lcdSendStr(" "); //send space
		}
		else {
			if (caracter==0x0d) {
				//lcdSendStr("cr");
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

/* Both work */
void test_read_UART(char *buffer, size_t buffer_size) {
	clear_Buffer(buffer);
	char search[] = "OK\r\n";
	char search2[] = "ERROR";
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		buffer[i] = caracter;
		i++;
		
		if (i >= buffer_size - 1) {
			break;
		}
		
		if (caracter == '\n') {
			char *ok = strstr(buffer, search);
			char *error = strstr(buffer, search2);
			//if (ok || error) {
				//commandOK = ok != NULL;
				//commandERROR = error != NULL;
				//break;
			//}
			
			/*To only show OK or ERROR*/
			if (ok) {
				commandOK = true;
				commandERROR = false;
				clear();
				lcdSendStr(ok);
				break;
			}
			if (error) {
				commandOK = false;
				commandERROR = true;
				clear();
				if(*(error - 1) == '\n'){
					lcdSendStr(error);
				}
				else{
					lcdSendStr(error-5);
				}
				break;
			}
		}
	}
	buffer[i] = '\0'; // Null terminate the string
}
void test_show_BUFF(char *buffer){
	int i = 0;
	char caracter=0x00;
	clear();
	while(1) {
		caracter = buffer[i];
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
