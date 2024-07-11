/* 
 * alligator.c
 * * * * * * * * * * * * * * * * * * * * *
 * * * * C O N T R O L  B G 9 5 M 3  * * *
 * * * * * * * * * * * * * * * * * * * * *
 *
 * Created: 01/04/2024 08:08:44 a. m.
 * Author : JAHH
 */ 

#ifndef F_CPU
#define F_CPU 9216000UL
#endif

#include "allinone.h"
#include "state_machine.h"
#include "bg95_mqtt.h" //debug quitar al final
#include "MXC4005XC.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

extern void init_modules(void);

volatile int cntTM = 0; //important as volatile? try non volatile
volatile int cntTE = 0;
ISR(WDT_vect)
{
	WDTCSR |= (1<<WDIF);
	cntTM++;
	cntTE++;
	
	if (cntTM==91) //92
	{
		cntTM = 0;
		estado = muestreo;
	}
	
	if (cntTE==125) //125
	{
		cntTE = 0;
		estado = envio;
	}
}
//ISR PARA SM FAKE FUNCIONA MASOMENOS
/*
extern int puerta;
ISR(WDT_vect)
{
	puerta++; //uncomment when state machine fake
	WDTCSR |= (1<<WDIF);
	cntTM++;
	cntTE++;
	
	if (cntTM==46) //92
	{
		cntTM = 0;
		estado = muestreo;
	}
	
	if (cntTE==110) //220
	{
		cntTE = 0;
		estado = envio;
	}
}
*/

void DrvGPIO_Init(){
	DDRB = 0b00101111; // xtal2, xtal1, pwrpt, magnetoR, nc, nc, nc, nc
	DDRC = 0b00001111; // ne, rst_mcu, scl, sda, resetBG95, powerBGB5, nc. nc. nc. nc
	DDRD = 0b00000010; // nc, nc, nc, nc, nc, int, txd, rxd
	DDRE = 0b00000000; // ne, nc, nc, nc, nc, swd, pt19, swc
	PORTB = 0b00010000; // x, x, out_off, in_pu, x, x, x, x
	PORTC = 0b01000000; // x, cp, cp, cp, out_off, out_off, x, x
	PORTD = 0b00000111; // nc, nc, nc, nc nc, in-pu, out_hi, in-pu
}

int main(void)
{
	DrvGPIO_Init(); //debug
	DrvSYS_Init();
	DrvUSART_Init();
	DrvTWI_Init();
	
	cntTM = 0;
	cntTE = 0;
	estado = dormido;
	
	u8 u8Reg;
	u8Reg = PMCR | (WDT_WCLKS << 4);
	PMCR = 0x80;
	PMCR = u8Reg;
	asm("cli"); //__disable_interrupt();
	asm("wdr");//__watchdog_reset();
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR =  0b11000100; // wdif - wdie - wdp3 - wdce - wde - wpd2 - wdp1 - wpd0
	
	
	
//////////////////////////////////////////////////////////////////////////
	//SMCR = 0x05; // modo = power down, habilita sleep DEBUG NO ENTRA A ISR CUANDO ACTIVO
	//_delay_ms(500);
	//PORTB = 0x00;
	//DDRD |= (0x01<<PORTD1);
	//PORTD = 0x02;
	//DDRC |= (1 << PORTC3); //POWER PORT
	////// Set PD2 (INT0) as input
	////DDRD &= ~(1 << PORTD2);		//try
//////////////////////////////////////////////////////////////////////////	
	//Salida para el buzzer
	DDRD |= (1<<DDD3);
	//Prueba buzzer
	PORTD |= (1<<PORTD3);
	_delay_ms(500);
	PORTD &= ~(1<<PORTD3);
	DDRD |= (0x01<<PORTD1); //igual
	DDRC |= (1 << PORTC3); //POWER PORT
	//// Set PD2 (INT0) as input
	//DDRD &= ~(1 << PORTD2);		//try
//////////////////////////////////////////////////////////////////////////
	
	
	
	//// FALLING EDGE TRIGGER (Weird and doesnt enter interrupt)
	//EICRA |= (1 << ISC01);
	//EICRA &= ~(1<< ISC00);
	////// RISING EDGE TRIGGER (might work too but its weird)
	////EICRA |= (1 << ISC01);
	////EICRA |= (1 << ISC00);
	////// ANY logic change (even worse)
	////EICRA |= (1 << ISC00);
	//////// LOW LEVEL TRIGGER (Enters interrupt with accelerometer but weird values)
	////EICRA &= ~(1 << ISC01);
	////EICRA &= ~(1 << ISC00);
	//// Enable INT0 external interrupt
	//EIMSK |= (1 << INT0);
	
	_delay_ms(1000);
	MXC4005XC_init(); //debug new
	sei(); // Enable global interrupts
	////EscribeMXC4005XC_NI(0x00, 0x08);
	////EscribeMXC4005XC_NI(0x0a, 0x08);
	
	////////// DEBUG FINAL REMOVE ALL THIS ///////////
	bg95_On(); //debug new
	bg95_init(); //TODO: meter el init en bg95_on! al final
	_delay_ms(15000);
	while(!mqtt_init());
	
	mqtt_pub_str("josepamb/feeds/welcome-feed", "---- START! ----");
	_delay_ms(1000);
	
	//mqtt_disconnect();
	DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
	_delay_ms(1000); //wait to complete turn off
	////////////////////////////////////////////////
	
	while(1){
		//computeStateMachine();
		computeStateMachine_fake();
	}
}
