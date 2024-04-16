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
#include <stddef.h>
#include "allinone.h"
#include "lcdi2c.h"
#include "DrvUSART.h"
#include "DrvSYS.h"
#include "state_machine.h"

State currentState = SENDING_AT_COMMANDS; //current state
extern char *MESSAGE;
extern char INBUFF[0x20];
extern bool data_received;

void initStateMachine(void) {
	currentState = SENDING_AT_COMMANDS;
}

void computeStateMachine(void) {
	switch (currentState){
		case IDLE: {
			//lcdSendStr("idle");
			//lcdSendStr(INBUFF); //debug: funciona pero mejorar
			if (!(PIND & (1 << 2))) { // PD2->1 (wait for button pressed)
				MESSAGE = "ATI\r\n"; //change message (debug)
				currentState = SENDING_AT_COMMANDS;
			}
			break;
		}
		case SENDING_AT_COMMANDS: {
			//lcdSendStr("SATC ");
			controlLED(true); // LED on while communicating
			_delay_ms(500);
			sendATCommands(MESSAGE);
			controlLED(false); // LED off when done
			currentState = READING_SENSORS;
			break;
		}	
		case READING_SENSORS: {
			//lcdSendStr("temp: ");
			//readTemperature();
			//char str[20];
			//sprintf(str,"%f",readTemperature());
			//lcdSendStr(str);
			//lcdSendStr("light: ");
			//readLight();
			
			currentState = READING_GPS;
			break;
		}	
		case READING_GPS: {
			//lcdSendStr("gps ");
			//MESSAGE = "ATI\r\n";
			//sendATCommands(MESSAGE);
			
			// ask for GPS coords from BG95 //
			//sendATCommands("ATI\r\n"); //OK
			//sendATCommands("AT+QGPS=1\r\n"); //OK
			//sendATCommands("AT+QGPSLOC?"); //GPS COORDS
			//char COORDS[0x60] = {0};
			//for (int i = 0; i < sizeof(INBUFF)/sizeof(INBUFF[0]); i++) {
				//COORDS[i] = INBUFF[i];
			//}
			// GNSS off: //
			//sendATCommands("AT+QGPSEND "); //OK
			
			currentState = IDLE; // Transition back
			break;
		}
		default:
		break;
	}
}

void sendATCommands(char *msg) {
	//DrvUSART_SendStr("at\r\n"); //debug quitar
	DrvUSART_SendStr(msg);
	
	//leeUART(); //maybe we need LF and CR, in that case use these functions instead
	//showBuff(); //debug: dejar
	
	//storeUART();
	//show_stored();
	
	new_read_UART();
	new_show_BUFF();
	//clear();
	//lcdSendStr(INBUFF);
}

//NOT STORING LF AND CR
void storeUART() {
	int i = 0;
	char caracter;
	//bool sequenceFound = false;
	char receivedBuffer[4] = {0}; // Buffer to hold the last four received characters
	while (1) {
		caracter = DrvUSART_GetChar(); //aqui cambia la bandera data_received //poner una interrupcion
		if (data_received) { // Check if data is available
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
			// iterate until receiving "OK\r\n"
			receivedBuffer[0] = receivedBuffer[1];
			receivedBuffer[1] = receivedBuffer[2];
			receivedBuffer[2] = receivedBuffer[3];
			receivedBuffer[3] = caracter;
			if (receivedBuffer[0] == 'O' &&	receivedBuffer[1] == 'K' &&	receivedBuffer[2] == '\r' && receivedBuffer[3] == '\n') {
				data_received = false;
				break;
			}
		}
		else {
			lcdSendStr("No data");
			break; // to not get stuck
		}
	}
	INBUFF[i] = '\0'; // null terminate INBUFF to treat as string
}
//NOT SHOWING LF AND CR
void show_stored(){
	int i = 0;
	char caracter=0x00;
	char receivedBuffer[4] = {0}; // Buffer to hold the last four received characters
	clear();
	while (1) {
		caracter = INBUFF[i];
		i++;
		lcdSendChar(caracter);
		// iterate until receiving "OK"
		receivedBuffer[0] = receivedBuffer[1];
		receivedBuffer[1] = caracter;
		if (receivedBuffer[0] == 'O' &&	receivedBuffer[1] == 'K') {
			data_received = false;
			break;
		}
	}
}

//with LF and CR
void leeUART() {
	int i = 0;
	char caracter;
	char receivedBuffer[4] = {0}; // Buffer to hold the last four received characters
	while (1) {
		caracter = DrvUSART_GetChar(); //aqui cambia la bandera data_received //poner una interrupcion
		if (data_received) { // Check if data is available
			INBUFF[i] = caracter;
			i++;
			// iterate until receiving "OK\r\n"
			receivedBuffer[0] = receivedBuffer[1];
			receivedBuffer[1] = receivedBuffer[2];
			receivedBuffer[2] = receivedBuffer[3];
			receivedBuffer[3] = caracter;
			if (receivedBuffer[0] == 'O' &&	receivedBuffer[1] == 'K' &&	receivedBuffer[2] == '\r' && receivedBuffer[3] == '\n') {
				data_received = false;
				break;
			}
		}
		else {
			lcdSendStr("No data");
			break; // to not get stuck
		}
	}
	//INBUFF[i] = '\0'; // null terminate INBUFF to treat as string
}
//with LF and CR
void showBuff(){
	int i = 0;
	char caracter=0x00;
	char receivedBuffer[4] = {0}; // Buffer to hold the last four received characters
	clear();
	while (1) {
		caracter = INBUFF[i];
		i++;
		if (caracter==0x0a) {
			lcdSendStr("lf"); //debug poner
		}
		else {
			if (caracter==0x0d) {
				lcdSendStr("cr"); //debug poner
			}
			else {
				lcdSendChar(caracter);
			}
		}
		// iterate until receiving "OK\r\n"
		receivedBuffer[0] = receivedBuffer[1];
		receivedBuffer[1] = receivedBuffer[2];
		receivedBuffer[2] = receivedBuffer[3];
		receivedBuffer[3] = caracter;
		if (receivedBuffer[0] == 'O' &&	receivedBuffer[1] == 'K' &&	receivedBuffer[2] == '\r' && receivedBuffer[3] == '\n') {
			data_received = false;
			break;
		}
	}
}

void printBuff(char *buff){
	int i = 0;
	char caracter=0x00;
	char receivedBuffer[4] = {0}; // Buffer to hold the last four received characters
	clear();
	while (1) {
		caracter = buff[i];
		i++;
		if (caracter==0x0a) {
			lcdSendStr("lf"); //debug poner
		}
		else {
			if (caracter==0x0d) {
				lcdSendStr("cr"); //debug poner
			}
			else {
				lcdSendChar(caracter);
			}
		}
		// iterate until receiving "OK\r\n"
		receivedBuffer[0] = receivedBuffer[1];
		receivedBuffer[1] = receivedBuffer[2];
		receivedBuffer[2] = receivedBuffer[3];
		receivedBuffer[3] = caracter;
		if (receivedBuffer[0] == 'O' &&	receivedBuffer[1] == 'K' &&	receivedBuffer[2] == '\r' && receivedBuffer[3] == '\n') {
			data_received = false;
			break;
		}
	}
}


/* WORKS PERFECT! */
void new_read_UART() {
	char search[] = "OK\r\n";
	char search2[] = "ERROR\r\n";
	//char search3[] = "ERROR "; //when the error has a code we need to store
	
	int i = 0;
	char caracter;
	while (1) {
		caracter = DrvUSART_GetChar();
		//if (caracter == 0x0A) { // Line feed
			//// Do nothing, skip storing it
		//}
		//else if (caracter == 0x0D) { // Carriage return
			//// Do nothing, skip storing it
		//}
		//else {
			//INBUFF[i] = caracter;
			//i++;
		//}
		INBUFF[i] = caracter;
		i++;
		// iterate until finding "OK or ERROR"
		char *ok = strstr(INBUFF, search);
		char *error = strstr(INBUFF, search2);
		if (ok || error) {
			break;
		}
	}
	INBUFF[i] = '\0';
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
			lcdSendStr("lf"); //debug poner
		}
		else {
			if (caracter==0x0d) {
				lcdSendStr("cr"); //debug poner
			}
			else {
				if (caracter == '\0') {
					//lcdSendStr("si");
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

// MAIN ORIGINAL!!! FUNCIONA!!!!!
/*
#ifndef F_CPU
#define F_CPU 9216000UL
#endif

#include "allinone.h" // del builder
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include "oled.h"
#include "lcdi2c.h"
#include "DrvUSART.h"
#include "DrvSYS.h"

// Import external definitions
extern void init_modules(void);

// Variable globales
char INBUFF[0x20] = {0};
bool data_received = false; // flag if communication is done

// Prototipo
void leeUART(void);
void showBuff(void);

int main(void)
{
	DrvSYS_Init(); // clock
	DrvUSART_Init(); //  Inicializa USART
	DrvTWI_Init(); // Inicializa  modulo i2c
	lcd_inicio();	// Inicializa pantalla LCD con i2c
	//lcd_init(LCD_DISP_ON); // Inicia OLED
	// Global interrupt enable
	//p SEI();
	DDRD |= (0x01<<PORTD1); //OUT PD1
	PORTD = 0x06; //High PD1 y PD2
	DDRB = 0xff; //OUT PB
	//PORTB = 0x01;
	_delay_ms(500);
	//PORTB = 0x00;
	
	clear(); // Limpia LCD
	lcdSendStr("BALA");
	//FillDisplay(0x00);		//ssd1306_lcd_clrscr();	// Limpia OLED
	//
	//oledPutString("BALATRON", 0, 10);
	//oledPutString("PRUEBA BG95", 1, 0);
	//oledPutString("ENVIA UN COMANDO", 2, 0);
	//oledPutString("0123456789!@#$%^&*()-", 3, 0);
	//

	while (1)
	{
		DrvUSART_SendStr("at\r\n");
		leeUART();
		lcdSendStr(INBUFF);
		//showBuff();
		_delay_ms(500);
	}
	return 0;
}

void leeUART(){
	int i=0;
	char contLF=2;
	char caracter;
	while(contLF){
		caracter = DrvUSART_GetChar();
		if (caracter==0x0a) // Si caracter es igual a line feed
		{
			contLF--;
			INBUFF[i]=caracter;
			i++;
		}
		else
		{
			INBUFF[i]=caracter;
			i++;
		}
		
	}
}

void showBuff(){
	int i=0;
	int contLF=2;
	char caracter=0x00;
	clear();
	while(contLF)
	{
		caracter = INBUFF[i];
		if (caracter==0x0a)
		{
			contLF--;
			lcdSendStr("lf");
		}
		else
		{
			if (caracter==0x0d)
			{
				lcdSendStr("cr");
			}
			else
			{
				lcdSendChar(caracter);
			}
		}
		i++;
	}
}
*/