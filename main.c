/* 
 * alligator.c
 * * * * * * * * * * * * * * * * * * * * *
 * * * * C O N T R O L  B G 9 5 M 3  * * *
 * * * * * * * * * * * * * * * * * * * * *
 *
 * Created: 01/04/2024 08:08:44 a. m.
 * Author : JAHH
 
 * Primera solucion para controlar el modulo BG95 M3
 *
 * Etapa 1:
 * 01/04/24	Activar OLED ssd1306 para depurar las respuestas del modulo BG95
 * Se unen los archvos generados por el builder LGT con un proyecto anterior con OLED
 * Se implementa una maquina de estados para mandar y recibir comandos con el BG95
 */ 

// STATE MACHINE!!!! //
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
#include "DrvSYS.h" //debug (add)
#include <stdio.h>

// global variables
typedef enum {
	// states
	IDLE,
	SENDING_AT_COMMANDS,
	PROCESSING_RESPONSES,
	READING_SENSORS,
	READING_GPS
} State;
State currentState = SENDING_AT_COMMANDS; //does one cycle, then stops at IDLE
char *MESSAGE; //format: "at\r\n"
char INBUFF[0x20] = {0};
bool data_received = false; // flag if communication is done

// Function prototypes
extern void init_modules(void); //external definitions
void leeUART(void);
void showBuff(void);
void printBuff(char *);
void storeUART(void);
void show_stored(void);
float readTemperature(void);
void readLight();
void readGPS(void);
void sendATCommands(char *);
void controlLED(bool state);
void pruebaOLED(void);

int main(void)
{
	DrvSYS_Init(); // Initialize system clock //debug (add)
	DrvUSART_Init(); // Initialize USART
	DrvTWI_Init(); // Initialize I2C
	lcd_inicio(); // Initialize LCD with I2C
	//SEI(); //enable interrupts
	//pruebaOLED(); //debug
	
	DDRD |= (0x01<<PORTD1); //OUT PD1
	PORTD = 0x06; //High PD1 y PD2
	DDRB = 0xff; //OUT PB
	_delay_ms(500);

	MESSAGE = "at\r\n";
	
	clear(); // clear LCD
	lcdSendStr("BALATRON");
	
	while (1)
	{
		switch (currentState){
			//// Idle state: wait for a button press or other event ////
			case IDLE:
			{	
				//lcdSendStr("idle");
				//lcdSendStr(INBUFF); //debug: funciona pero mejorar
				if (!(PIND & (1 << 2))) { // PD2->1 (wait for button pressed)
					MESSAGE = "ATI\r\n"; //change message (debug)
					currentState = SENDING_AT_COMMANDS;
				}
				break;
			}
			
			//// 1 Sending AT commands state (first state) ////
			case SENDING_AT_COMMANDS:
			{
				//lcdSendStr("SATC ");
				controlLED(true); // LED on while communicating
				_delay_ms(500);
				sendATCommands(MESSAGE);
				controlLED(false); // LED off when done
				currentState = READING_SENSORS;
				break;
			}
			
			//// 2 Reading temperature state ////
			case READING_SENSORS:
			{
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
				
			//// 3 Reading GPS state ////
			case READING_GPS:
			{
				//lcdSendStr("gps ");
				MESSAGE = "ATI\r\n";
				sendATCommands(MESSAGE);
				
				//ask for GPS coords from BG95
				//sendATCommands("ATI\r\n"); //OK
				//sendATCommands("AT+QGPS=1\r\n"); //OK
				//sendATCommands("AT+QGPSLOC?"); //GPS COORDS
				char COORDS[0x20] = {0};
				for (int i = 0; i < sizeof(INBUFF)/sizeof(INBUFF[0]); i++) {
					COORDS[i] = INBUFF[i];
				}
				//printBuff(COORDS);
				//lcdSendStr(COORDS);
				
				// GNSS off:
				//sendATCommands("AT+QGPSEND "); //OK
				
				currentState = IDLE; // Transition back
				break;
			}

			default:
			break;
		}
		_delay_ms(100);
	}
	return 0;
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

///// las originales /////
/*
 void leeUART(){ //solo lee la primera linea (pensar como leer mas de una linea)
	 int i=0;
	 char contLF=2; //hay varias lineas
	 char caracter;
	 //debug: cambiar a while(still_receiving) o hasta que deje de encontrar lf
	 // puede ser un bool de que siga encontrando o no caracters
	 while(contLF){
		 caracter = DrvUSART_GetChar();
		 if (caracter==0x0a) // Si caracter es igual a line feed
		 {
			 contLF--;
			 INBUFF[i]=caracter; //no guardar LF //debug
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
			lcdSendStr("lf"); //debug poner
		}
		else
		{
			if (caracter==0x0d)
			{
				lcdSendStr("cr"); //debug poner
			}
			else
			{
				////debug: para no imprimir comando enviado
				//if((caracter != 0x61) && (caracter != 0x74) && (caracter != 0x69)){
				//lcdSendChar(caracter); //debug: no imprimira comando enviado
				//} //debug quitar
				lcdSendChar(caracter);
			}
		}
		i++;
	}
}
*/
 
float readTemperature() {
	float temp = 32.5; //read temperature data from a sensor
	return temp;
}

void readLight() {
	//read light data from a sensor
}

void sendATCommands(char *msg) {
	//DrvUSART_SendStr("at\r\n"); //debug quitar
	DrvUSART_SendStr(msg);
	//leeUART(); //maybe we need LF and CR, in that case use these functions instead
	//showBuff(); //debug: dejar
	storeUART();
	show_stored();
}

void controlLED(bool state) {
	if (state) {
		PORTB |= (1 << 0); // Turn on PB0 LED
	}
	else {
		PORTB &= ~(1 << 0); // Turn off PB0 LED
	}
}

void pruebaOLED(){
	lcd_init(LCD_DISP_ON); // Inicia OLED
	//ssd1306_lcd_clrscr();	// Limpia OLED
	FillDisplay(0x00);
	//_delay_ms(500);
	oledPutString("BALATRON", 0, 10);
	oledPutString("PRUEBA BG95", 1, 0);
	oledPutString("INBUFF:", 2, 0);
	
}
