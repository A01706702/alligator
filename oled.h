/*
 * oled.h
 *
 * Created: 25/01/2021 11:28:44 p. m.
 *  Author: anton
 */ 


#ifndef OLED_H_
#define OLED_H_

#include <inttypes.h>
#include <avr/pgmspace.h>
//#include "usi_i2c_master.h"

#define LCD_DISP_OFF        0xAE
#define LCD_DISP_ON        0xAF
#define DISPLAY_WIDTH        128
#define DISPLAY_HEIGHT        64
#define LCD_I2C_ADR         0x78	//(0x7a >> 1)
#define NORMALSIZE 1
#define DOUBLESIZE 2

// De microchip
// Defines the display offset in x direction. Dependent on the display
// used and how it is connected.
#define OFFSET	2

//#define FONT            ssd1306oled_font
//#ifndef YES
//#define YES        1
//#endif
	void tst_comando(char comando); // prueba i2c con comando
	void tst_data(char dato);
	void setCP(void); //set address page and address column
    void lcd_command(uint8_t cmd[], uint8_t size);    // transmit command to display
    void lcd_data(uint8_t data[], uint16_t size);    // transmit data to display
    void lcd_init(uint8_t dispAttr);
    void lcd_home(void);                            // set cursor to 0,0
    void lcd_invert(uint8_t invert);        // invert display
    void lcd_sleep(uint8_t sleep);            // display goto sleep (power off)
    void lcd_set_contrast(uint8_t contrast);    // set contrast for display
    void lcd_puts(const char* s);            // print string, \n-terminated, from ram on screen (TEXTMODE)
    // or buffer (GRAPHICMODE)
    void lcd_puts_p(const char* progmem_s);        // print string from flash on screen (TEXTMODE)
    // or buffer (GRAPHICMODE)
    
    void ssd1306_lcd_clrscr(void);                // clear screen (and buffer at GRFAICMODE)
    void lcd_putc(char c);                // print character on screen at TEXTMODE
	
	//void oledWriteChar1x(char letter, unsigned char page, unsigned char column);
	void oledPutString(char *ptr,unsigned char page, unsigned char col);
	void FillDisplay(unsigned char data);

	
#endif /* OLED_H_ */