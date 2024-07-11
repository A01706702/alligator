#ifndef _USART_H_
#define _USART_H_

#include <avr/interrupt.h>

#define E_UMSEL0_UART	0	/**< Asynchronous USART */
#define E_UMSEL0_USRT	1	/**< Synchronous USART */
#define E_UMSEL0_SPIS	2	/**< Slave SPI */
#define E_UMSEL0_SPIM	3	/**< Master SPI */

enum emUpm0
{
	E_UPM0_DIS = 0x0,	/**< Disable Parity */
	E_UPM0_EVEN = 0x2,	/**< Enable, Even Parity */
	E_UPM0_ODD			/**< Enable, Odd Parity */
};

enum emUsbs0
{
	E_USBS0_1B,			/**< 1 Stop Bit */
	E_USBS0_2B,			/**< 2 Stop Bit */
};

enum emUcsz0
{
	E_UCSZ0_5B,			/**< USART Character Size: 5-bit */
	E_UCSZ0_6B,			/**< USART Character Size: 6-bit */
	E_UCSZ0_7B,			/**< USART Character Size: 7-bit */
	E_UCSZ0_8B,			/**< USART Character Size: 8-bit */
	E_UCSZ0_9B = 0x7	/**< USART Character Size: 9-bit */
};

enum emUcpol0
{
	E_UCPOL0_TRE_SFE,	/**< Transmit Data on Rising XCK1 Edge, 
							Sample Data on Falling XCK1 Edge */							
	E_UCPOL0_TFE_SRE,	/**< Transmitted Data on Falling XCK1 Edge, 
							Sample Data on Rising XCK1 Edge */
};

#ifndef _DRVUSART_SRC_
#endif
	
void DrvUSART_Init(void);
//void DrvUSART_SendChar(u8 u8Char); //cleaning
void DrvUSART_SendStr(const char* str);
void processData(char *buff, size_t buffsize);
void processData_wait(char *buff, size_t buffsize, int timeout_ms);

#endif