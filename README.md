
# BeaconCell

## Description
IOT cellular beacon implementation using BG95 and LGT8F328P

## Functionalities

- GNSS location
- MQTT (SSL) cloud communication
- UART between chip and LTE module
- Motion detection (MXC4005XC)

## Implementation
State machine control

![state machine](fsm.jpeg)

## Public Methods

### StateMachine.c

* [void computeStateMachine(void)](#computeStateMachine)
* [void sendATCommands(char *)](#sendATCommands)
* [void iluminacion(void)](#iluminacion)
* [void temperatura(void)](#temperatura)
* [void GPS(void)](#GPS(void))
* [void clear_Buffer(char *, size_t)](#clear_Buffer)
* [void print_Buffer(char *, size_t)](#print_Buffer)
* [void TRYING_GPS(char *)](#TRYING_GPS)
* [void TRY_COMMAND(char *, char *, size_t)](#TRY_COMMAND)
* [bool handle_Response(char *buffer, size_t buffersize)](#handle_Response)
* [bool handleErrorCode(char *, size_t)](#handleErrorCode)
* [void RETRY_COMMAND(int, char *, char *, size_t)](#RETRY_COMMAND)
* [int toggleValue(int)](#toggleValue)

### DrvUSART.c

* [void DrvUSART_Init(void)](#Init)
* [void DrvUSART_SendChar(u8 u8Char)](#DrvUSART_SendChar)
* [void DrvUSART_SendStr(char* str)](#DrvUSART_SendStr)
* [u8 DrvUSART_GetChar(void)](#DrvUSART_GetChar)
* [void DrvUSART_GetString(void)](#DrvUSART_GetString)
* [void processData(char *, size_t)](#processData)
* [void appendSerial(char)](#appendSerial)
* [void serialWrite(char *)](#serialWrite)


### MXC4005XC.c

* [void MXC4005XC_init(void)](#MXC4005XC_init)
* [u8 LeeMXC4005XC_NI(u8 regAddr)](#LeeMXC4005XC_NI)
* [void EscribeMXC4005XC_NI(u8 regAddr, u8 data)](#EscribeMXC4005XC_NI)
* [void MXC4005XC_GetData_test(float *data)](#MXC4005XC_GetData_test)
* [void MXC4005XC_GetData_real(float *data)](#MXC4005XC_GetData_real)
* [float MXC4005XC_Get_Temperature(void)](#MXC4005XC_Get_Temperature)
* 

### ErrorHandling.c

* [bool handleMoveOn(void)](#handleMoveOn)
* [bool handleRetry(void)](#handleRetry)
* [bool handleNoErrorCode(void)](#handleNoErrorCode)
* [bool handle505(void)](#handle505)
* [bool handleconnection(char *buffer, size_t buffersize)](#handleconnection)


### bg95MQTT.c

* [bool mqtt_init(void)](#mqtt_init)
* [void mqtt_pub_str(const char *topic, const char *message)](#mqtt_pub_str)
* [void mqtt_pub_char(const char *topic, const char message)](#mqtt_pub_char)
* [void mqtt_pub_unsigned_short(const char *topic, unsigned short message)](#mqtt_pub_unsigned_short)
* [void mqtt_pub_float(const char *topic, const float message)](#mqtt_pub_float)
* [void mqtt_pub_int(const char *topic, const int message)](#mqtt_pub_int)
* [void mqtt_disconnect(void)](#mqtt_disconnect)


# Public Methods - Extension

## DrvUSART.c

1. ### Init
	* Autogen USART registers and ports initializer. Check macros.h for enabling/disabling interruptions
```
 example code here
```
2. ### DrvUSART_SendChar
	&nbsp;&ensp;&ensp; ***NOT USING INTERRUPTION***
	* Busy waits until USART data register is empty, then writes received char into UDR0
3. ### DrvUSART_SendStr
	&nbsp;&ensp;&ensp; ***NOT USING INTERRUPTION***
	* Receives array pointer and for each char it calls DrvUSART_SendChar.
4. ### DrvUSART_GetChar
	&nbsp;&ensp;&ensp; ***NOT USING INTERRUPTION***
	* Busy waits until USARTs data register is full with unread data, then returns UDR0
5. ### DrvUSART_GetString
	&nbsp;&ensp;&ensp; ***USING INTERRUPTION***
	* Reads circular RX buffer and prints each char on the LCD. This method updates rxReadPos on the circular buffer rxBuffer.
6. ### processData
	&nbsp;&ensp;&ensp; ***USING INTERRUPTION***
	* IMPORTANT: uncomment ptr lines for proper working, right now its commented for debugging purposes
	* Function to handle responses with and without echoed command.
	* Receives pointer to linear array (char) and fills it with zeros
	* Pointer to first char of actual received response in rxBuffer
	* compares pointer to last Command, if last Command was found it skips it
	* Copies each char into received array and updates rxReadPos.
7. ### appendSerial
	&nbsp;&ensp;&ensp; ***USING INTERRUPTION***
	* Fills circular TX Buffer with received char to transmit
8. ### serialWrite
	&nbsp;&ensp;&ensp; ***USING INTERRUPTION***
	* Receives array pointer and for each char it calls appendSerial.
	* Sends dummy byte to trigger interruption just in case

## state_machine.c

1.  ### computeStateMachine
	* call it to enter state machine
```
example code here
```
2. ### sendATCommands
	* For testing and debugging, not used much. It calls DrvUSART_SendStr to send a command and DrvUSART_GetString to print it on the LCD
3. ### iluminacion
	* Obtains the light read from ALS-PT19 sensor 
4. ### temperatura
	* not implemented yet
5. ### GPS
	* Enters location data retrieving routine, enables GNSS, then calls the function TRY_COMMAND to try and get coordinates multiple times
6. ### clear_Buffer
	* fills an array with zeros using memset
7. ### print_Buffer
	* calls lcdSendChar for each char in an array to print on LCD
8. ### TRY_COMMAND
	* receives a command and an array to store response, tries a command multiple times and handles specific errors if failed (not implemented yet). If command was successful it breaks
9. ### handle_Response
	* OK or ERROR handling with switch case. Will be changed to dictionary structure for error specific
10. ### handleErrorCode
	* different implementation of handle response with struct and error codes array
11. ### TRYING_GPS
	* TRY_COMMAND specific to GPS using COORDS buffer
12. ### RETRY_COMMAND
	* Different implementation for TRY_COMMAND using recursion
13. ### toggleValue
	* Toggles the received int between 0 and 1 using XOR. Useful for flags.



## MXC4005XC.c

1. ### MXC4005XC_init
	* Initializes the Interrupt port to falling edge and powers up accelerometer setting interrupt mask, detection and sensitivity.
2. ### LeeMXC4005XC_NI
	* I2C Read routine specific for MXC4005XC.
3. ### EscribeMXC4005XC_NI
	* I2C Write routine specific for MXC4005XC
4. ### MXC4005XC_GetData_real
	* Retrieves and processes accelerometer data from the MXC4005XC sensor, returning the actual accelerations and temperature in Celsius.
5. ### MXC4005XC_Get_Temperature
	* Reads and returns only the temperature data from the MXC4005XC sensor.


## ErrorHandling.c
1. ### handleMoveOn
	* Returns True.
2. ### handleRetry
	* Returns False.
3. ### handleNoErrorCode
	* handles ERROR according to lastCommand for all types of errors (without code) including mqtt.
4. ### handle505
	* Handles GPS turning on or off.
5. ### handleconnection
	* Handles connection errors.


## bg95MQTT.c
1. ### mqtt_init
	* LTE priority, SSL, PDP and MQTT Open and Connect.
2. ### mqtt_pub_str
	* Publishes a string message to the specified MQTT topic.
3. ### mqtt_pub_char
	* Publishes a character message to the specified MQTT topic.
4. ### mqtt_pub_unsigned_short
	* Publishes an unsigned short message to the specified MQTT topic.
5. ### mqtt_pub_float
	* Publishes a float message to the specified MQTT topic. Note: manual making of a float (integer part and fractional part).
6. ### mqtt_pub_int
	* Publishes an integer message to the specified MQTT topic.
7. ### mqtt_disconnect
	* Disconnects from the MQTT broker and deactivates context.
