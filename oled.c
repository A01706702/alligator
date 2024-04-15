/*
 * oled.cpp
 *
 * Created: 25/01/2021 11:24:30 p. m.
 *  Author: anton
 */ 
#include "allinone.h"
#include <string.h>
#include "oled.h"
#include "font.h"

//static uint8_t charMode = NORMALSIZE;
//static uint8_t displayBuffer[DISPLAY_HEIGHT/8][DISPLAY_WIDTH];


// Lista de comandos para ajustar la pagina y columna de SSD1306
const uint8_t dlist1[] PROGMEM  = {
	0x22,	//SSD1306_PAGEADDR,
	0x00,   // Page start address
	0x07,//0x02,   // Page end (not really, but works here)
	0x21,	//SSD1306_COLUMNADDR, // Column start address
	0
};


// Envía un comando a SSD1306
void tst_comando(char comm)
{
	DrvTWI_SendStart();
	DrvTWI_WaitForComplete();
	
	DrvTWI_SendByte(LCD_I2C_ADR);//( deviceAddr & 0xFE )
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(LCD_I2C_ADR << 1);
	
	DrvTWI_SendByte(0x00);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(0x00);
	
	DrvTWI_SendByte(comm);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(comm);
	
	DrvTWI_SendStop();
	while( !(inb(TWCR) & BV(TWSTO)) );//i2c_driver_stop();
	
}

// Envía un dato al SSD1306
void tst_data(char d)
{
	
	DrvTWI_SendStart();
	DrvTWI_WaitForComplete();//i2c_driver_start();
	DrvTWI_SendByte(LCD_I2C_ADR);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata((LCD_I2C_ADR << 1) | 0);
	
	DrvTWI_SendByte(0x40);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(0x40);
	
	DrvTWI_SendByte(d);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(d);
	
	DrvTWI_SendStop();
	while( !(inb(TWCR) & BV(TWSTO)) );//i2c_driver_stop();
	
}


// Ajusta la columna y pagina
void setCP(void)
{
	uint8_t commandSequence[sizeof(dlist1)+1];
	for (uint8_t i = 0; i < sizeof (dlist1); i++) {
		commandSequence[i] = (pgm_read_byte(&dlist1[i]));
	}
	commandSequence[sizeof(dlist1)]=(127);
	lcd_command(commandSequence, sizeof(commandSequence));
}


// Lista de comandos en memoria de programa para iniciar SSD1306	
const uint8_t init_sequence [] PROGMEM = {    // Initialization Sequence
	LCD_DISP_OFF,   // Display OFF (sleep mode)
	0x20, 0b10,		// Ajusta al modo de direccionamiento horizontal
					// 00=Horizontal Addressing Mode
					// 01=Vertical Addressing Mode
					// 10=Page Addressing Mode (RESET)
					// 11=Invalid
	//0x21,0x00,0x7f,					
	//0x22,			// Ajusta direccion de la pagina
	//0x00,			// pagina inicial
	//0x07,			// pagina final
	0xB0,            // Set Page Start Address for Page Addressing Mode, 0-7
	0xC8,            // Set COM Output Scan Direction
	0x00,            // --set low column address
	0x10,            // --set high column address
	0x40,            // --set start line address
	0x81, 0x3F,      // Set contrast control register
	0xA1,            // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA6,            // Set display mode. A6=Normal; A7=Inverse
	0xA8, DISPLAY_HEIGHT-1, // Set multiplex ratio(1 to 64)
	0xA4,            // Output RAM to Display
	// 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
	0xD3, 0x00,      // Set display offset. 00 = no offset
	0xD5,            // --set display clock divide ratio/oscillator frequency
	0xF0,            // --set divide ratio
	0xD9, 0x22,      // Set pre-charge period
	// Set com pins hardware configuration
	#if DISPLAY_HEIGHT==64
	0xDA, 0x12,
	#elif DISPLAY_HEIGHT==32
	0xDA, 0x02,
	#endif
	0xDB,            // --set vcomh
	0x20,            // 0x20,0.77xVcc
	0x8D, 0x14,      // Set DC-DC enable
};


// Envía un conjunto de comandos al ssd1036
void lcd_command(uint8_t cmd[], uint8_t size) {
	
	DrvTWI_SendStart();
	DrvTWI_WaitForComplete();
	
	DrvTWI_SendByte(LCD_I2C_ADR);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(LCD_I2C_ADR << 1);
	
	DrvTWI_SendByte(0x00);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(0x00);
	
	for (uint8_t i=0; i<size; i++)
	{
		DrvTWI_SendByte(cmd[i]);
		DrvTWI_WaitForComplete();//i2c_driver_TXdata(cmd[i]);
	}
	
	DrvTWI_SendStop();
	while( !(inb(TWCR) & BV(TWSTO)) );//i2c_driver_stop();
		
}

// Envia un conjunto de datos al ssd1036
void lcd_data(uint8_t data[], uint16_t size) {
	
	DrvTWI_SendStart();
	DrvTWI_WaitForComplete();
	
	DrvTWI_SendByte(LCD_I2C_ADR);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata((LCD_I2C_ADR << 1) | 0);
	
	DrvTWI_SendByte(0x40);
	DrvTWI_WaitForComplete();//i2c_driver_TXdata(0x40);
	
	for (uint16_t i = 0; i<size; i++) {
		
		DrvTWI_SendByte(data[i]);
		DrvTWI_WaitForComplete();//i2c_driver_TXdata(data[i]);
	}
	DrvTWI_SendStop();
	while( !(inb(TWCR) & BV(TWSTO)) );//i2c_driver_stop();
	
}

// Inicialización de SDD1306
void lcd_init(uint8_t dispAttr){
	uint8_t commandSequence[sizeof(init_sequence)+1];
	for (uint8_t i = 0; i < sizeof (init_sequence); i++) {
		commandSequence[i] = (pgm_read_byte(&init_sequence[i]));
	}
	commandSequence[sizeof(init_sequence)]=(dispAttr);
	lcd_command(commandSequence, sizeof(commandSequence));
}


void lcd_putc(char c){
	int base;
	base = (c-0x30) * 48;
	for(int j = 0; j < 48; j++)
	{
		c = pgm_read_byte(&basicfont[base++]);
		tst_data(c);
	}
}

void lcd_puts(const char* s){
	while (*s) {
		lcd_putc(*s++);
	}
}

void ssd1306_lcd_clrscr(){
	uint8_t lista_comm[]={
		0x22,	//SSD1306_PAGEADDR,
		0x00,   // Page start address
		0x07,   // Page end (not really, but works here)
		0x21,	//SSD1306_COLUMNADDR, // Column start address
		0,127
		};
		
	lcd_command(lista_comm, sizeof(lista_comm)); // envia comandos para ajustar PAGE y COLUMN

	for(int i=0; i<1024; i++)
	{
		tst_data(0x00);
	}
}


void oledWriteChar1x(char letter, unsigned char page, unsigned char column)
{
	char i;

	letter -= ' ';					// Adjust character to table that starts at 0x20
	tst_comando(page);
	column += OFFSET;
	tst_comando(0x00+(column&0x0F));
	tst_comando(0x10+((column>>4)&0x0F));

//c = pgm_read_byte(&basicfont[base++]);

	tst_data(pgm_read_byte(&basicfont[letter][0]));	// Write first column
	tst_data(pgm_read_byte(&basicfont[letter][1]));	// Write second column
	tst_data(pgm_read_byte(&basicfont[letter][2]));	// Write third column
	tst_data(pgm_read_byte(&basicfont[letter][3]));	// Write fourth column
	tst_data(pgm_read_byte(&basicfont[letter][4]));	// Write fifth column
	tst_data(0x00);					// Write 1 column for buffer to next character
	return;
}

void oledPutString(char *ptr,unsigned char page, unsigned char col)
{
	unsigned char i;
	
	i=col;
	page = page + 0xB1;
	while(*ptr)
	{
		oledWriteChar1x(*ptr,page,i);
		ptr++;
		i+=6;
	}
}


void FillDisplay(unsigned char data)
{
	unsigned char i,j;

	for(i=0xB0;i<0xB8;i++)			// Go through all 8 pages
	{
		tst_comando(i);		// Set page
		tst_comando(0x00+OFFSET);		// Set lower column address
		tst_comando(0x10);		// Set upper column address

		for(j=0;j<132;j++)			// Write to all 132 bytes
		{
			tst_data(data);
		}
	}
	return;
}