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

// MAIN SIMPLIFIED //
#ifndef F_CPU
#define F_CPU 9216000UL
#endif

#include "allinone.h" // del builder
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "oled.h"
#include "lcdi2c.h"
#include "DrvUSART.h"
#include "DrvSYS.h"
#include "state_machine.h"

// global variables
char *MESSAGE; //format: "at\r\n"
char INBUFF[0x40] = {0}; //change size (?)
bool data_received = false; // flag if communication is done

// Function prototypes
extern void init_modules(void); //external definitions
void pruebaOLED(void);

int main(void)
{
	DrvSYS_Init(); // Initialize system clock //debug (add)
	DrvUSART_Init(); // Initialize USART
	DrvTWI_Init(); // Initialize I2C
	lcd_inicio(); // Initialize LCD with I2C
	//SEI(); //enable interrupts
	
	DDRD |= (0x01<<PORTD1); //OUT PD1
	PORTD = 0x06; //High PD1 y PD2
	DDRB = 0xff; //OUT PB
	_delay_ms(500);

	MESSAGE = "ati\r\n";
	clear(); // clear LCD
	lcdSendStr("BALATRON");
	initStateMachine();
	
	while (1)
	{
		computeStateMachine();
		//// searching in string
		//char search[] = "ati\r\r\nQuectel\r\nBG95-M3\r\nRevision: BG95M3LAR02A03\r\n\r\nOK\r\n";
		//char *ptr = strstr(INBUFF, search);
		//if (ptr != NULL) {
			//clear();
			//lcdSendStr("yea");
		//}
		//else {
			//clear();
			//lcdSendStr("nel");
		//}
		
		//iterating through a string
		//char *str = "An example.";
		//size_t i = 0;
		//while (str[i] != '\0') {
			//lcdSendChar(str[i]);
			//i++;
		//}
		_delay_ms(100);
	}
	return 0;
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