/*
 * state_machine.h
 *
 * Created: 16/04/2024
 *  Author: JPMB
 */ 

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdbool.h>

typedef enum {
	IDLE,
	SENDING_AT_COMMANDS,
	PROCESSING_RESPONSES,
	READING_SENSORS,
	READING_GPS
} State;
extern State currentState; // declared in state_machine.c

void initStateMachine(void);
void computeStateMachine(void);
void controlLED(bool state);
void sendATCommands(char *msg);
float readTemperature(void);
void readLight(void);
void readGPS(void);

void storeUART(void);
void show_stored(void);
void leeUART(void);
void showBuff(void);
void printBuff(char *);

void new_read_UART(void);
void new_show_BUFF(void);

#endif /* STATE_MACHINE_H_ */