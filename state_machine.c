/*
 * state_machine.c
 *
 * Created: 16/04/2024
 *  Author: JPMB
 */ 

#ifndef F_CPU
#define F_CPU 9216000UL
#endif

#include "allinone.h"
#include "state_machine.h"
#include "error_handling.h"
#include "bg95_mqtt.h"
#include "MXC4005XC.h"

////These must be global:
u16 light = 0x0000;		//2 bytes
float temper = 0.0;		//4 bytes
int puerta = 0;		//2 bytes
char COORDS[34] = "no hay nada";	//34 bytes
float ACCEL_BUFF[4] = {0}; //debug remove

extern char lastCommand[20]; //20 bytes

ERROR errorActions[] = { //ERROR codes handling
	//CME ERROR CODES
	{0, handleMoveOn},		// Phone failure, continue
	{3, handleMoveOn},		// Operation not allowed, continue
	{4, handleMoveOn},		// Operation not supported, continue
	{30, handleMoveOn},		// No network service, continue
	//GNSSS ERROR CODES
	{504, handleMoveOn},	//Session is ongoing, continue
	{505, handle505},		//Session not active, try last command again
	{506, handleMoveOn},	//Operation timeout, continue
	{507, handleMoveOn},	//Function  not enabled, continue
	{516, handleRetry},		//No fix, try again //debug retry
	{549, handleMoveOn},	//Unknown error, continue
};
/*
ERROR mqttURCsActions[] = {
	//MQTT URCs
	{1, handleMoveOn},			//  TODO: handlemqttconnection
	{2, handleMoveOn},			//  TODO: Deactivate PDP first, and then activate PDP and reopen MQTT connection.
	{3, handleMoveOn},			//	TODO:
									//1. Check whether the inputted user name and	password are correct.
									//2. Make sure the client ID is not used.
									//3. Reopen MQTT connection and try to send CONNECT packet to the server again.}
	
	{4, handleMoveOn},			//	TODO:
									//1. Check whether the inputted user name and	password are correct.
									//2. Make sure the client ID is not used.
									//3. Reopen MQTT connection and try to send CONNECT packet to the server again.}

	{5, handleMoveOn},			//  Normal, move on
	{6, handleMoveOn},			//	TODO:
									//1. Make sure the data is correct.
									//2. Try to reopen MQTT connection since there may be network congestion or an error.

	{7, handleMoveOn},			//  TODO: Make sure the link is alive or the server is available currently.
};
*/

//SM FINAL (WORKS ALMOST PERFECT!):
void computeStateMachine(void) {
	switch(estado)
	{
		case dormido:
			//DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			//_delay_ms(1000); //wait to turn off
			break;
		
		case muestreo:
			bg95_On(); //turn on
			bg95_init();
			temper = MXC4005XC_Get_Temperature();
			//temperature = ACCEL_BUFF[3]; //another option
			light = iluminacion();
			if(!GPS())
			{
				//estado = reparando; //debug new
				//break;
			}
			
			DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			_delay_ms(1000); //wait to turn off
			estado = dormido;
			break;
		
		case envio: //mandar a la nube
			bg95_On(); //turn on
			bg95_init();
			if(mqtt_init()){
				mqtt_pub_float("josepamb/feeds/beacon.temperature", temper); //send temperature (float)
				_delay_ms(1000);
				mqtt_pub_unsigned_short("josepamb/feeds/beacon.light", light); //send light data (uint16_t or unsigned short)
				_delay_ms(2000);
				mqtt_pub_int("josepamb/feeds/beacon.puerta", puerta); //debug
				_delay_ms(2000);
				mqtt_pub_str("josepamb/feeds/beacon.gps/csv", COORDS); //send GPS buffer (string)
				_delay_ms(2000);
			}
			//else{
				//estado = reparando;
				//break;
			//}
			
			//mqtt_disconnect();
			_delay_ms(2000);
			DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			_delay_ms(1000); //wait to turn off
			estado = dormido;
			break;
			
		case reparando:
			//wait until fix or send with a timeout of 5 minutes
			mqtt_disconnect();
			_delay_ms(2000);
			mqtt_init();
			
			break;
		
		case movimiento:
			////u8 interrupt_source = LeeMXC4005XC_NI(MXC4005XC_REG_INT_SRC0); //read INT source register 0x00
			////TODO: ver si se abrio o cerro...
			//puerta++; //contar las veces que se abrio
			//
			//DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			//_delay_ms(1000); //wait to turn off
			
			estado = dormido;
			break;
		
		default:
			estado = dormido;
			break;
	}
	asm("sleep");
	asm("nop");
	asm("nop");
}

//TEST uncomment puerta line in WDT ISR
/*
void computeStateMachine_fake(void) {
	switch(estado)
	{
		case dormido:
			break;
			
		case muestreo:
			bg95_On(); //turn on
			bg95_init();
			
			temper = MXC4005XC_Get_Temperature();
			light = iluminacion();
			if(!GPS())
			{
				//////mqtt_pub_str("josepamb/feeds/welcome-feed", "GPS mal!");
				//////_delay_ms(1000);
			}
			
			DrvUSART_SendStr("AT+QPOWD=1");
			_delay_ms(1000);
			
			estado = dormido;
			break;
		
		case envio:
			bg95_On(); //turn on
			bg95_init();
			
			if(mqtt_init()){
				mqtt_pub_int("josepamb/feeds/beacon.puerta", puerta);
				_delay_ms(1000);
				mqtt_pub_str("josepamb/feeds/beacon.gps/csv", COORDS); //send GPS buffer (string)
				_delay_ms(1000);
			}
			//else{
				////estado = arreglando;
				////break;
			//}
			//mqtt_disconnect();
			
			DrvUSART_SendStr("AT+QPOWD=1");
			_delay_ms(1000);
			
			estado = dormido;
			break;
			
		case movimiento:
			//mqtt_pub_str("josepamb/feeds/welcome-feed", "movimiento");
			//_delay_ms(1000);
						
			puerta++;
			
			estado = dormido;
			break;
		
		default:
			estado = dormido;
			break;
	}
	asm("sleep");
	asm("nop");
	asm("nop");
}
*/
//TEST Timer y estados (fake muestreo).
/*
void computeStateMachine_fake(void) {
	switch(estado)
	{
		case dormido:
			break;
			
		case muestreo:
			bg95_On(); //turn on
			bg95_init();
			if(mqtt_init()){
				mqtt_pub_int("josepamb/feeds/welcome-feed", 11);
				_delay_ms(1000);
			}

			//mqtt_disconnect();
			//_delay_ms(2000);
			
			DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			_delay_ms(1000); //wait to turn off
			
			estado = dormido;
			break;
		
		case envio:
			bg95_On(); //turn on
			bg95_init();
			if(mqtt_init()){
				mqtt_pub_int("josepamb/feeds/welcome-feed", 2222);
				_delay_ms(1000);
			}

			//mqtt_disconnect();
			//_delay_ms(2000);
			
			DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			_delay_ms(1000); //wait to turn off
			
			estado = dormido;
			break;
			
		case movimiento:
			estado = dormido;
			break;
		
		default:
			estado = dormido;
			break;
	}
	asm("sleep");
	asm("nop");
	asm("nop");
}
*/
//MQTT ONLY IN ENVIO (sin apagar bg95):
/*
void computeStateMachine_fake(void) {
	//mainly for testing accelerometer (movimiento)
	switch(estado)
	{
		case dormido:
			break;
			
		case muestreo:
			//temper = MXC4005XC_Get_Temperature();
			light = iluminacion();
			temper++;
			if(!GPS())
			{
				//////mqtt_pub_str("josepamb/feeds/welcome-feed", "GPS mal!");
				//////_delay_ms(1000);
			}
			estado = dormido;
			break;
		
		case envio:
			if(mqtt_init()){
				mqtt_pub_float("josepamb/feeds/beacon.temperature", temper);
				_delay_ms(1000);
				//mqtt_pub_float("josepamb/feeds/beacon.temperature", ACCEL_BUFF[3]);
				//_delay_ms(1000);
				mqtt_pub_float("josepamb/feeds/beacon.light", light);
				_delay_ms(1000);
				mqtt_pub_int("josepamb/feeds/beacon.puerta", puerta); //debug
				_delay_ms(2000);
				mqtt_pub_str("josepamb/feeds/beacon.gps/csv", COORDS); //send GPS buffer (string)
				_delay_ms(1000);
			}
			//else{
				////mqtt_disconnect();
				////estado = arreglando;
				////break;
			//}
			mqtt_disconnect();
			
			estado = dormido;
			break;
			
		case movimiento:
			//mqtt_pub_str("josepamb/feeds/welcome-feed", "movimiento");
			//_delay_ms(1000);
						
			puerta++;
			
			estado = dormido;
			break;
		
		default:
			estado = dormido;
			break;
	}
	asm("sleep");
	asm("nop");
	asm("nop");
}
*/
//SIN APAGAR MQTT NI BG95:
void computeStateMachine_fake(void) {
	//mainly for testing accelerometer (movimiento)
	switch(estado)
	{
		case dormido:
			break;
			
		case muestreo:
			mqtt_pub_str("josepamb/feeds/welcome-feed", "muestreo");
			_delay_ms(1000);
			//temper = MXC4005XC_Get_Temperature();
			//light = iluminacion();
			temper++;
			light++;
			puerta++;
			
			if(!GPS())
			{
				//////mqtt_pub_str("josepamb/feeds/welcome-feed", "GPS mal!");
				//////_delay_ms(1000);
			}
			estado = dormido;
			break;
		
		case envio:
			mqtt_pub_float("josepamb/feeds/beacon.temperature", temper);
			_delay_ms(1000);
			//mqtt_pub_float("josepamb/feeds/beacon.temperature", ACCEL_BUFF[3]);
			//_delay_ms(1000);
			mqtt_pub_float("josepamb/feeds/beacon.light", light);
			_delay_ms(1000);
			mqtt_pub_int("josepamb/feeds/beacon.puerta", puerta); //debug
			_delay_ms(2000);
			mqtt_pub_str("josepamb/feeds/beacon.gps/csv", COORDS); //send GPS buffer (string)
			_delay_ms(1000);
			
			estado = dormido;
			break;
			
		case movimiento:
			//mqtt_pub_str("josepamb/feeds/welcome-feed", "movimiento");
			//_delay_ms(1000);
						
			puerta++;
			
			estado = dormido;
			break;
		
		default:
			estado = dormido;
			break;
	}
	asm("sleep");
	asm("nop");
	asm("nop");
}
//Prende y solo manda estado
/*
void computeStateMachine_fake(void)  {
	switch(estado)
	{
		case dormido:
			//DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			//_delay_ms(1000); //wait to turn off
			break;
		
		case muestreo:
			bg95_On(); //turn on
			bg95_init();
			
			while(!mqtt_init());
			mqtt_pub_str("josepamb/feeds/welcome-feed", "muestreo");
			_delay_ms(1000);
			
			//mqtt_disconnect();
			//_delay_ms(2000);
			DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			_delay_ms(1000); //wait to turn off
			estado = dormido;
			break;
		
		case envio: //mandar a la nube
			bg95_On(); //turn on
			bg95_init();
			
			while(!mqtt_init());
			mqtt_pub_str("josepamb/feeds/welcome-feed", "envio");
			_delay_ms(1000);
			
			//mqtt_disconnect();
			//_delay_ms(2000);
			DrvUSART_SendStr("AT+QPOWD=1"); //apagar bg95
			_delay_ms(1000); //wait to turn off
			estado = dormido;
			break;
			
		case reparando:
			//wait until fix or send with a timeout of 5 minutes
			mqtt_disconnect();
			_delay_ms(2000);
			mqtt_init();
			
			break;
		
		case movimiento:
			estado = dormido;
			break;
		
		default:
			estado = dormido;
			break;
	}
	asm("sleep");
	asm("nop");
	asm("nop");
}
*/

u16 iluminacion(void){
	// Lee la iluminacion ambiental dentro del equipo (ADC6 <- ALS-PT19)
	u16 luz;
	PORTB |= (1<<PORTB5); //Energiza ALS-PT19
	DrvADC_Init(); //Inicializa ADC
	_delay_us(500);
	luz = DrvADC_readChannel(0x06);
	// Para depuracion, descomentar 5 lineas:
	//char a, b;
	//a=ADCL;
	//b=ADCH;
	//DrvUSART_SendChar(a);
	//DrvUSART_SendChar(b);
	ADCSRA &= ~(1 << ADEN); //Deshabilita modulo ADC
	PORTB &= ~(1 << PORTB5); // desenergiza ALS-PT19
	//TODO: almacenar o promediar o notificar
	return luz;
}
void bg95_On(void){
	PORTC |= (1 << PORTC3);
	_delay_ms(600);
	PORTC &= ~(1 << PORTC3);
	_delay_ms(5000); // until led blinks
	//aqui meter todo lo de init
}
void bg95_init(void){
	char TEMP[128] = {0}; //for responses
	TRY_COMMAND("ATE0", TEMP, sizeof(TEMP)); //Desactivar el eco
	TRY_COMMAND("AT&W0", TEMP, sizeof(TEMP)); //guardar configuracion (NO eco)
	TRY_COMMAND("AT+COPS=0", TEMP, sizeof(TEMP));
	TRY_COMMAND("AT+QGPSXTRA=1", TEMP, sizeof(TEMP)); //already activated can remove
	
	//TRY_COMMAND("AT+QICSGP=1,3,\"internet.itelcel.com\",\"\",\"\",0",TEMP, sizeof(TEMP)); //TELCEL APN
	//TRY_COMMAND("AT+QICSGP=1,3,\"m2m.tele2.com\",\"\",\"\",0",TEMP, sizeof(TEMP)); //TELE2 APN
	TRY_COMMAND("AT+QICSGP=1,3,\"internet.oxio.com\",\"\",\"\",0",TEMP, sizeof(TEMP)); //OXIO APN
	
	//WWAN PRIORITY, HTTP context 1 CFG and SSL context 0 CFG
	TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1;+QHTTPCFG=\"contextid\",1;+QHTTPCFG=\"sslctxid\",1;+QSSLCFG=\"sslversion\",1,4;+QSSLCFG=\"ciphersuite\",1,0xFFFF;+QSSLCFG=\"seclevel\",1,0;+QSSLCFG=\"sslversion\",0,4;+QSSLCFG=\"ciphersuite\",0,0xFFFF;+QSSLCFG=\"seclevel\",0,0;+QSSLCFG=\"ignorelocaltime\",0,1", TEMP, sizeof(TEMP));
	////GPS PRIORITY
	//TRY_COMMAND("AT+QGPSCFG=\"priority\",0,1;+QHTTPCFG=\"contextid\",1;+QHTTPCFG=\"sslctxid\",1;+QSSLCFG=\"sslversion\",1,4;+QSSLCFG=\"ciphersuite\",1,0xFFFF;+QSSLCFG=\"seclevel\",1,0;+QSSLCFG=\"sslversion\",0,4;+QSSLCFG=\"ciphersuite\",0,0xFFFF;+QSSLCFG=\"seclevel\",0,0;+QSSLCFG=\"ignorelocaltime\",0,1", TEMP, sizeof(TEMP));
	//TRY_COMMAND("AT+QGPS=1", TEMP, sizeof(TEMP)); //para ir obteniendo fix con XTRA
	
	//if no file or expired wait until extra downloaded: +QGPSURC: "XTRA_DL",0
	
}

//WORKING: with altitude:
bool GPS(void) {
	char TEMP[128] = {0};
	const char *ptr;
	const char *lat_start;
	const char *lon_start;
	const char *alt_start;
	size_t lat_len, lon_len, alt_len;
	
	TRY_COMMAND("AT+QGPSCFG=\"priority\",0,0", TEMP, sizeof(TEMP)); //debug GPS
	_delay_ms(1000);
	TRY_COMMAND("AT+QGPS=1", TEMP, sizeof(TEMP)); // Turn on GPS
	//tries 5 times:
	for(int i = 0; i<5; i++){
		if(TRY_COMMAND("AT+QGPSLOC=2", TEMP, sizeof(TEMP))){
			if ((ptr = strstr(TEMP, "+QGPSLOC:")) != NULL) {
				//EXAMPLE RESPONSE:
				//			<UTC>,		<lat>,	 <lon>,	 <HDOP>,<altitude>,<fix>,<COG>,<spkm>,<spkn>,<date>,<nsat>
				//+QGPSLOC: 202153.000,20.64651,-100.43036,1.1,1824.5,3,0.00,0.0,0.0,180624,06
				
				////TRY_COMMAND("AT+QGPSEND", TEMP, sizeof(TEMP)); // turn GPS off??
				//TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1", TEMP, sizeof(TEMP)); //WWAN
				//_delay_ms(1000);
				
				ptr += strlen("+QGPSLOC: ");
				ptr = strchr(ptr, ','); //skip utc
				if (!ptr) break; //DEBUG QUITAR PUERTA VALORES DE AQUI
				ptr++;

				lat_start = ptr;
				ptr = strchr(ptr, ',');
				if (!ptr) break;
				lat_len = ptr - lat_start;
				ptr++;
				
				lon_start = ptr;
				ptr = strchr(ptr, ',');
				if (!ptr) break;
				lon_len = ptr - lon_start;
				ptr++;
				
				ptr = strchr(ptr, ','); // skip HDOP
				if (!ptr) break;
				ptr++;
				
				alt_start = ptr;
				ptr = strchr(ptr, ',');
				if (!ptr) break;
				alt_len = ptr - alt_start;
				
				// Manually construct the COORDS string
				if (lat_len + lon_len + alt_len + 3 < sizeof(COORDS)) {
					int len = snprintf(COORDS, sizeof(COORDS), "%d,", puerta);
					strncpy(COORDS + len, lat_start, lat_len);
					len += lat_len;
					COORDS[len++] = ',';
					strncpy(COORDS + len, lon_start, lon_len);
					len += lon_len;
					COORDS[len++] = ',';
					strncpy(COORDS + len, alt_start, alt_len);
					len += alt_len;
					//snprintf(COORDS + len, sizeof(COORDS) - len, ",0"); //not needed anymore?
					
					TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1", TEMP, sizeof(TEMP)); //WWAN
					_delay_ms(1000);
					return true;
				}
			}
		}
		//cli(); //debug quitar al final porque desincroniza WDTimer.
		_delay_ms(3000); //wait for fix
		//sei();
	}
	
	TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1", TEMP, sizeof(TEMP)); //WWAN
	_delay_ms(1000);
	snprintf(COORDS, sizeof(COORDS), "%d - no location found", puerta); //debug remove //se tarda 18 segundos en llegar aqui
	
	////if no first fix:
	//if (strstr(COORDS, "no hay nada") != NULL) {
		//snprintf(COORDS, sizeof(COORDS), "%d, - no location found", puerta); //debug remove //se tarda 18 segundos en llegar aqui
	//}
	////if there has been a fix before, store the last found location:
	//else {
		//// Temporarily store the current location data, skipping the initial number and comma
		//char tempLocation[27];
		//char *commaPos = strchr(COORDS, ','); // Find the first comma
		//if (commaPos != NULL) {
			//strncpy(tempLocation, commaPos + 1, sizeof(tempLocation) - 1);
			//tempLocation[sizeof(tempLocation) - 1] = '\0'; // Ensure null-termination
//
			//// Update COORDS with the new puerta value and the existing location data
			//snprintf(COORDS, sizeof(COORDS), "%d,%s", puerta, tempLocation);
		//}
	//}
	return false; //no fix
}
//WORKING: without altitude:
/*
bool GPS(void) {
	char TEMP[128] = {0};
	const char *ptr;
	const char *lat_start;
	const char *lon_start;
	size_t lat_len, lon_len;
	
	TRY_COMMAND("AT+QGPSCFG=\"priority\",0,0", TEMP, sizeof(TEMP)); //debug GPS
	_delay_ms(1000);
	TRY_COMMAND("AT+QGPS=1", TEMP, sizeof(TEMP)); // Turn on GPS
	
	for(int i = 0; i<5; i++){
		if(TRY_COMMAND("AT+QGPSLOC=2", TEMP, sizeof(TEMP))){
			if ((ptr = strstr(TEMP, "+QGPSLOC:")) != NULL) {
				//EXAMPLE RESPONSE:
				//+QGPSLOC: 202153.000,20.64651,-100.43036,1.1,1824.5,3,0.00,0.0,0.0,180624,06
				
				////TRY_COMMAND("AT+QGPSEND", TEMP, sizeof(TEMP)); // turn GPS off??
				//TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1", TEMP, sizeof(TEMP)); //WWAN
				//_delay_ms(1000);
				
				ptr += strlen("+QGPSLOC: ");
				ptr = strchr(ptr, ',');
				if (!ptr) break;
				ptr++;

				lat_start = ptr;
				ptr = strchr(ptr, ',');
				if (!ptr) break;
				lat_len = ptr - lat_start;
				ptr++;
				lon_start = ptr;
				ptr = strchr(ptr, ',');
				if (!ptr) break;
				lon_len = ptr - lon_start;
				
				// Manually construct the COORDS string
				if (lat_len + lon_len + 2 < sizeof(COORDS)) {
					int len = snprintf(COORDS, sizeof(COORDS), "%d,", puerta);
					strncpy(COORDS + len, lat_start, lat_len);
					len += lat_len;
					COORDS[len++] = ',';
					strncpy(COORDS + len, lon_start, lon_len);
					len += lon_len;
					snprintf(COORDS + len, sizeof(COORDS) - len, ",0");
					TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1", TEMP, sizeof(TEMP)); //WWAN
					_delay_ms(1000);
					return true;
				}
			}
		}
		_delay_ms(3000);
	}
	
	TRY_COMMAND("AT+QGPSCFG=\"priority\",1,1", TEMP, sizeof(TEMP)); //WWAN
	_delay_ms(1000);
	snprintf(COORDS, sizeof(COORDS), "%d - no fix", puerta); //debug remove
	return false; //no fix
}
*/


void sendATCommand(const char* command){
	char TEMP[128] = {0};
	DrvUSART_SendStr(command);
	processData_wait(TEMP, sizeof(TEMP), 10000); //10 sec
}
//void wait_response(const char* command, const char* expected_response){
	//char TEMP[128] = {0};
	//DrvUSART_SendStr(command);
	//processData_wait(TEMP, sizeof(TEMP), 20000);
	////while(1){
		////processData_wait(TEMP, sizeof(TEMP), 0);
		////if (strstr(TEMP, expected_response) != NULL) {
			////break;
		////}
	////}
//}

/* WORKS PERFECT: Try command and handle response */
bool TRY_COMMAND(const char *command, char *buffer, size_t buffersize){
	DrvUSART_SendStr(command);
	processData(buffer, buffersize);
	//processData_wait(buffer, buffersize, 3000); //3 second tolerance for every command
	
	if (strstr(buffer, "OK") != NULL) {
		//if last command is Network related and it is NOT QMTCFG:
		if ((strstr(lastCommand, "AT+QMTOPEN") != NULL) ||
		(strstr(lastCommand, "AT+QMTCONN") != NULL) ||
		(strstr(lastCommand, "AT+QCELL") != NULL) ||
		(strstr(lastCommand, "AT+QHTTPGET") != NULL)){
			if ((strstr(buffer, "+QMT") == NULL) ||
			(strstr(buffer, "+QCELL") == NULL) ||
			(strstr(buffer, "+QHTTP") == NULL)) { //if buffer does NOT contain Network response
				processData_wait(buffer, buffersize, 20000); //wait for +QMT response
			}
			//return true;
			return handleconnection(buffer, buffersize);
		}
		return true; //if lastcommand is NOT network related
	}
	else if(strstr(buffer, "CONNECT") != NULL) {
		return true;
	}
	//return true;
	return handleError(buffer, buffersize);
}
bool handleError(char *buffer, size_t buffersize) {
	char *errorptr = strstr(buffer, "ERROR");
	//char *urcptr = strstr(buffer, "QMTSTAT"); //if response has "+QMTSTAT..."
	if (errorptr != NULL) {
		if (errorptr[5] == ':') { //if error has error code
			int errorCode = atoi(errorptr + strlen("ERROR: "));
			for (size_t i = 0; i < sizeof(errorActions) / sizeof(errorActions[0]); i++) {
				if (errorActions[i].code == errorCode) {
					return errorActions[i].action();
				}
			}
		}
		else { //redundant else
			return handleNoErrorCode(); //just "ERROR" without code
		}
	}
	//else if(urcptr != NULL){ //mqtt URCs
		//int errorCode = atoi(urcptr + strlen("QMTSTAT: 0,")); //obtain error code
		//for (size_t i = 0; i < sizeof(mqttURCsActions) / sizeof(mqttURCsActions[0]); i++) {
			//if (mqttURCsActions[i].code == errorCode) {
				//return mqttURCsActions[i].action(); //handle specific error code
			//}
		//}
	//}
	return true; // No "ERROR" nor "OK", nor error code found, buffer is empty (shouldnt enter here ever.)
}

//Location using cellular test 2 working perfect
/*
bool cell_location(void) {
	char TEMP[128] = {0};
	char API_URL[128] = {0};
	const char *ptr = TEMP;
	unsigned int mcc = 0;     // 3 digits
	unsigned int mnc = 0;     // 2 digits
	unsigned int tac = 0;     // Hexadecimal, 2 bytes
	unsigned long cellid = 0;  // Hexadecimal, 28 bits (4 bytes)

	//----------- STEP 1: Get network information -------------//
	if (!TRY_COMMAND("AT+QENG=\"servingcell\"", TEMP, sizeof(TEMP))) {
		return false;
	}

	ptr = TEMP;
	//while if i ever want to change logic to weighted approach (using AT+CELLSCAN)
	while ((ptr = strstr(ptr, "+QENG: ")) != NULL) {
		ptr += strlen("+QENG: ");
		if (strstr(ptr, "\"GSM\"") != NULL) {
			// Parse GSM cell information
			if (sscanf(ptr, "\"servingcell\",%*[^,],\"GSM\",%u,%u,%x,%lx", &mcc, &mnc, &tac, &cellid) != 4) {
				return false;
			}
			} else if (strstr(ptr, "\"eMTC\"") != NULL) {
			// Parse eMTC cell information
			if (sscanf(ptr, "\"servingcell\",%*[^,],\"eMTC\",%*[^,],%u,%u,%lx,%*d,%*d,%*d,%*d,%*d,%x", &mcc, &mnc, &cellid, &tac) != 4) {
				return false;
			}
		}
		// Move to the next line
		ptr = strchr(ptr, '\n');
		if (ptr == NULL) {
			break;
		}
		ptr++;
	}
	
	//----------- STEP 3: Use Geolocation API to approximate location for each tower -------------//
	TRY_COMMAND("AT+QIACT=1", TEMP, sizeof(TEMP));
	TRY_COMMAND("AT+QSSLCFG=\"sni\",1,1", TEMP, sizeof(TEMP)); //important
	
	
	//NOTE: AJAX url seems more accurate on initial tests...
	snprintf(API_URL, sizeof(API_URL), "https://opencellid.org/ajax/searchCell.php?&mcc=%u&mnc=%u&lac=%u&cell_id=%lu",
	mcc, mnc, tac, cellid);
	//snprintf(API_URL, sizeof(API_URL), "https://opencellid.org/cell/get?key=pk.7d121714a5e01aac0f3ae816c18b0991&mcc=%u&mnc=%u&lac=%u&cellid=%lu&format=at",
	//mcc, mnc, tac, cellid);
	//// https://opencellid.org/ajax/searchCell.php?&mcc=334&mnc=20&lac=23312&cell_id=58224897
	char command[24];
	snprintf(command, sizeof(command), "AT+QHTTPURL=%d,10", strlen(API_URL));
	if (!TRY_COMMAND(command, TEMP, sizeof(TEMP))) {
		return false;
	}
	
	DrvUSART_SendStr(API_URL);
	
	if (!TRY_COMMAND("AT+QHTTPGET=80", TEMP, sizeof(TEMP))) {
		return false;
	}

	if (!TRY_COMMAND("AT+QHTTPREAD=80", TEMP, sizeof(TEMP))) {
		return false;
	}
	
	if ((ptr = strstr(TEMP, "\"lon\":")) != NULL) {
		ptr += strlen("\"lon\":\"");
		const char *lon_start = ptr;
		while (*ptr != '"' && *ptr != '\0') {
			ptr++;
		}
		if (*ptr == '\0') {
			return false; // Malformed JSON
		}
		size_t lon_len = ptr - lon_start;

		if((ptr = strstr(ptr, "\"lat\":\"")) == NULL){
			return false; // Malformed JSON
		}
		ptr += strlen("\"lat\":\""); // length of "\"lat\":\""
		const char *lat_start = ptr;
		while (*ptr != '"' && *ptr != '\0') {
			ptr++;
		}
		if (*ptr == '\0') {
			return false; // Malformed JSON
		}
		size_t lat_len = ptr - lat_start;

		// Manually construct the COORDS string
		if (lon_len + lat_len + 6 < sizeof(COORDS)) {
			int len = snprintf(COORDS, sizeof(COORDS), "%d,", puerta);
			strncpy(COORDS + len, lat_start, lat_len);
			len += lat_len;
			COORDS[len++] = ',';
			strncpy(COORDS + len, lon_start, lon_len);
			len += lon_len;
			snprintf(COORDS + len, sizeof(COORDS) - len, ",0");
			return true;
		}
		else {
			return false; // Not enough space in COORDS buffer
		}
	}
	return false;
}
*/