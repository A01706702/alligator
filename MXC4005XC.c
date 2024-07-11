/*
 * MXC4005XC.c
 *
 * Created: 07/05/2024 01:10:27 p. m.
 *  Author: JPMB
 */
#include "allinone.h"
#include "MXC4005XC.h"
#include "state_machine.h"

#include "bg95_mqtt.h" //debug remove

/*
ISR(INT0_vect){
	//interrupcion de acelerometro INT pin
	//1 leer registro STATUS o INT_SRC0 podria ser aqui pero funcion para leer tiene busy waits
	//2 if cambio: estado = movimiento
	//3 escribir 1 en posicion de INT_CLR0 para hacer clear en esa especifica posicion
	// Toda la rutina se hace en "movimiento" para no atorar en ISR()
	
	//debug try this below?
	//u8 interrupt_source = LeeMXC4005XC_NI(MXC4005XC_REG_INT_SRC0); //read INT source register 0x00
	//EscribeMXC4005XC_NI(MXC4005XC_REG_INT_CLR0, interrupt_source); //clear INT source bits by writing a 1 in CLR
	
	//estado = movimiento; //aqui se hace todo lo de leer data, clear, etc (mas limpio creo)
	mqtt_pub_str("josepamb/feeds/welcome-feed", "se moviooo");
	//_delay_ms(1000);
}
*/
//int cont = 0;
extern int puerta;
ISR(INT0_vect){
	u8 valor = LeeMXC4005XC_NI(MXC4005XC_REG_INT_SRC0); // Lee fuente de interrupcion
	EscribeMXC4005XC_NI(MXC4005XC_REG_INT_CLR0, valor); //0xcf Borra fuente de interrpcion
	EscribeMXC4005XC_NI(MXC4005XC_REG_INT_CLR1, 0x01);
	
	
	//cont++;
	//mqtt_pub_int("josepamb/feeds/welcome-feed", cont); //debug
	
	puerta++;
	//mqtt_pub_int("josepamb/feeds/beacon.puerta", puerta); //debug
}


void MXC4005XC_init(void){
	// Configure INT0 (PD2) pin for falling edge trigger
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);
	EIMSK |= (1 << INT0); // Enable external interrupt INT0
	EscribeMXC4005XC_NI(MXC4005XC_REG_INT_MASK0, 0xcf); // Funciona con 0xcf solo si hay una inclinacion aprox de 45 grados del plano XY
	EscribeMXC4005XC_NI(MXC4005XC_REG_DETECTION, 0x80);
	
//////////////////////////////////////////////////////////////////////////

	//EscribeMXC4005XC_NI(MXC4005XC_REG_INT_MASK0, MXC4005XC_INT_MASK0_EN); //enable all INT_MASK0 Interrupts
	//EscribeMXC4005XC_NI(MXC4005XC_REG_INT_MASK1, MXC4005XC_INT_MASK1_EN); //enable all INT_MASK1 Interrupts //debug
	//
	EscribeMXC4005XC_NI(MXC4005XC_REG_CTRL, MXC4005XC_CMD_2G_POWER_ON); //initial setup 2g
	////EscribeMXC4005XC_NI(MXC4005XC_REG_DETECTION, MXC4005XC_DETECTION_PARAMS) //debug
}

void EscribeMXC4005XC_NI(u8 regAddr, u8 data){
	//1 START CONDITION
	cbi(TWCR, TWIE); // disable TWI interrupt
	DrvTWI_SendStart(); // send start condition
	DrvTWI_WaitForComplete();
	//2 DEVICE ADDRESS with WRITE (0)
	DrvTWI_SendByte(MXC4005XC_ADDRESS << 1 | 0); // Direccion i2c del MXC4005 con WRITE (2a)
	DrvTWI_WaitForComplete();
	//3 REGISTER ADDRESS
	DrvTWI_SendByte(regAddr); // Direccion del registro dentro del MXC4005
	DrvTWI_WaitForComplete();
	
	//4 DATA[7:0]
	DrvTWI_SendByte(data); // Escribir datos en registro especificado
	DrvTWI_WaitForComplete();
	
	// STOP CONDITION end of communication
	DrvTWI_SendStop();
	while (!(inb(TWCR)&BV(TWSTO)));
}

//Works for 1 register
u8 LeeMXC4005XC_NI(u8 regAddr){
	//1 START CONDITION
	cbi(TWCR, TWIE); // disable TWI interrupt
	DrvTWI_SendStart(); // send start condition
	DrvTWI_WaitForComplete();
	//2 DEVICE ADDRESS with WRITE (0)
	DrvTWI_SendByte(MXC4005XC_ADDRESS << 1 | 0); // Direccion i2c del MXC4005 con WRITE (2a)
	DrvTWI_WaitForComplete();
	//3 REGISTER ADDRESS
	DrvTWI_SendByte(regAddr); // Direccion del registro dentro del MXC4005
	DrvTWI_WaitForComplete();
	
	//4 START CONDITION AGAIN
	DrvTWI_SendStart(); // re envia start
	DrvTWI_WaitForComplete();
	//5 DEVICE ADDRESS AGAIN with READ (1)
	DrvTWI_SendByte(MXC4005XC_ADDRESS << 1 | 1); // Direccion i2c del MXC4005 con READ (2b)
	DrvTWI_WaitForComplete();
	//6 NOTACKNOWLEDGE
	DrvTWI_ReceiveByte(FALSE); // accept receive data and nack it (last-byte signal)
	DrvTWI_WaitForComplete();
	
	u8 valor = DrvTWI_GetReceivedByte(); //store data in variable
	
	//7 STOP CONDITION end of communication
	DrvTWI_SendStop();
	while (!(inb(TWCR)&BV(TWSTO)));
	return valor;
}

/*
*****************************************
** GET ALL DATA FUNCTIONS: DEBUG MAKE STATIC????
*****************************************
*/
//Should work: Stores all useful sensor data
void MXC4005XC_GetData_real(float *data)
{
	uint8_t data_reg[7] = {0}; // data buffer
	for (uint8_t i = 0; i < 7; i++){
		data_reg[i] = LeeMXC4005XC_NI(MXC4005XC_REG_DATA+i); //not great, better to create a function for multi reg reading
	}
	for (uint8_t i = 0; i < 3; i++) {
		data[i] = (float)((int16_t)(data_reg[i*2]<<8 | data_reg[i*2 + 1]) >> 4);
		data[i] /= MXC4005XC_2G_SENSITIVITY; // convert acceleration to g
	}
	data[3] = (float)data_reg[6] * MXC4005XC_T_SENSITIVITY + MXC4005XC_T_ZERO; // convert to Celsius //DEBUG compare to get temperature
}

//Should work
float MXC4005XC_Get_Temperature(void)
{
	//MXC4005_REG_TOUT
	/*MXC400xXC contains an on-chip temperature sensor whose output can be read through the I2C
	interface. The output is in 2's complement format. The nominal value of TOUT[7:0] is 0 at a 
	temperature of 25°C, and the sensitivity of the output is approximately 0.586°C/LSB.
	*/	
	
	//u8 raw_data = LeeMXC4005XC_NI(MXC4005_REG_TOUT);
	//int8_t val = (int8_t)raw_data; // cast into signed 8 bit int
	//val = ~val + 1; // re-apply 2's complement because response comes in that format
	//float temp = MXC4005XC_T_ZERO - (float)val * MXC4005XC_T_SENSITIVITY; //DEBUG sensitivity
	//return temp;
	
	u8 raw_data = LeeMXC4005XC_NI(MXC4005_REG_TOUT);
	int8_t val = (int8_t)raw_data; // cast into signed 8 bit int
	float temp = (float)val * MXC4005XC_T_SENSITIVITY + MXC4005XC_T_ZERO; //DEBUG sensitivity
	return temp;
}

//debug??
//u8 MXC4005XC_Get_Orientation(void)
//{
	//// MXC4005XC_REG_INT_SRC1 
	///*The ORXY[1:0] and ORZ[1:0] bits in INT_SRC1 indicate the orientation of MXC400xXC.
	//ORXY indicates orientation of the XY plane, and ORZ indicates direction of the Z axis.
	//Mapping of the orientation bits is as follows:
	//*/
	//u8 orxyz = LeeMXC4005XC_NI(MXC4005XC_REG_INT_SRC1);
	////TILT, ORZ, ORXY[1], ORXY[0], 0, 0, 0, DRDY
	//
	////TODO: consider the DRDY bit to check if new data is available //debug
	////if (DRDY bit is set (0x01)){
		//////new data is available in acceleration, go and read it
	////}
	//return orxyz;
//}


////Should work but debug
//void MXC4005XC_Get_Acceleration(u8 data[6])
//{
	////MXC4005_REG_XOUT_UPPER, etc
	///*XOUT[11:0] is the 12-bit X-axis acceleration output.
	//The output is in 2's complement format, with a range of ?2048 to +2047.
	//*/
	//data[0] = LeeMXC4005XC_NI(MXC4005_REG_XOUT_UPPER); //X upper
	//data[1] = LeeMXC4005XC_NI(MXC4005_REG_XOUT_LOWER); //X lower
	//data[2] = LeeMXC4005XC_NI(MXC4005_REG_YOUT_UPPER); //Y upper
	//data[3] = LeeMXC4005XC_NI(MXC4005_REG_YOUT_LOWER); //Y lower
	//data[4] = LeeMXC4005XC_NI(MXC4005_REG_ZOUT_UPPER); //Z upper
	//data[5] = LeeMXC4005XC_NI(MXC4005_REG_ZOUT_LOWER); //Z lower
	//for (uint8_t i = 0; i < 6; i++) {
		//data[i] = (float)data[i] / MXC4005XC_2G_SENSITIVITY; // convert acceleration to g
	//}
//}

/*
// debug ??
void LeeMuchos(u8 regAddr, int num, uint8_t *data){ //DEBUG
	//1 START CONDITION
	cbi(TWCR, TWIE); // disable TWI interrupt
	DrvTWI_SendStart(); // send start condition
	DrvTWI_WaitForComplete();
	//2 DEVICE ADDRESS with WRITE (0)
	DrvTWI_SendByte(MXC4005XC_ADDRESS << 1 | 0); // Direccion i2c del MXC4005 con WRITE (2a)
	DrvTWI_WaitForComplete();
	//3 REGISTER ADDRESS
	DrvTWI_SendByte(regAddr); // Direccion del registro dentro del MXC4005
	DrvTWI_WaitForComplete();

	//4 START CONDITION AGAIN
	DrvTWI_SendStart(); // re envia start
	DrvTWI_WaitForComplete();

	for (uint8_t i = 0; i < num; i++){
		//5 DEVICE ADDRESS AGAIN with READ (1)
		DrvTWI_SendByte(MXC4005XC_ADDRESS << 1 | 1); // Direccion i2c del MXC4005 con READ (2b)
		DrvTWI_WaitForComplete();

		//DrvTWI_ReceiveByte(TRUE);
		//DrvTWI_WaitForComplete();
		data[i] = DrvTWI_GetReceivedByte(); //store data in variable
	}

	//6 NOT ACKNOWLEDGE
	DrvTWI_ReceiveByte(FALSE); // accept receive data and nack it (last-byte signal)
	DrvTWI_WaitForComplete();

	//7 STOP CONDITION end of communication
	DrvTWI_SendStop();
	while (!(inb(TWCR)&BV(TWSTO)));
}

// debug ??
void MXC4005XC_GetData_test(float *data)
{
	//aqui juntar todas las funciones de data
	uint8_t data_reg[7] = {0};
	LeeMuchos(MXC4005XC_ADDRESS, MXC4005XC_REG_DATA, 7, data_reg);
	for (uint8_t i = 0; i < 3; i++) {
		data[i] = (float)((int16_t)(data_reg[i*2]<<8 | data_reg[i*2 + 1]) >> 4);
		data[i] /= MXC4005XC_2G_SENSITIVITY; // convert acceleration to g
	}
	data[3] = (float)data_reg[6] * MXC4005XC_T_SENSITIVITY + MXC4005XC_T_ZERO; // convert to Celsius
}
*/