// /*
// MAIN ORIGINAL!!! FUNCIONA!!!!!
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
char INBUFF[0x20];

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
	lcdSendStr("BALATRON INDUSTR");
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
		showBuff();
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