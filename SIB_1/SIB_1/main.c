/*
 * SIB_1.c
 * Created: 6/3/2017 9:56:57 AM
 * Author : Scott Szafranski
 */ 

#define F_CPU 16000000UL

#define TCA_W_ADD 0b11100000
#define TCA_R_ADD 0b11100001
#define MPL_W_ADD 0b11000000
#define MPL_R_ADD 0b11000001

#define RR_Press_ID 507
#define RR_Temp_ID 515

#define LEAK 1
#define DEPTH 2
#define GENERAL 3

#define Rx0ID  0x000
#define Rx1ID  0x000
#define Rx2ID  0x001
#define Rx3ID  0x001
#define Rx4ID  0x001
#define Rx5ID 0x001

#include "headers/defines2.h"		//Pin name definitions

/**************************************************************
	INCLUDES
***************************************************************/

#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>

#include "headers/SIB.h"
#include "headers/global.h"			//general define header pulled from net

#include "headers/functions.h"		//general functions
#include "headers/spi_ry.h"			//SPI protocol implementation

#include "headers/mcp2515_ry_def.h"	//MCP2515 register and bit definitions
#include "headers/mcp2515_ry.h"		//MCP2515 functions
#include "headers/can_frames.h"
#include "headers/usart_ry.h"		//serial communication with PC

#include "headers/uart.h"
#include "headers/ADC.h"

/******************************************************************************
	CANBUS ID definition|
******************************************************************************/
//RxID is your device ID that you allow messages to receive
uint16_t RxID = 0x03;	//SIB

//TxID is the target ID you're transmitting to
uint8_t TxID = 0x00;	//Tegra

/******************************************************************************
	CANBUS Related variabes
*******************************************************************************/
tCAN CANTX_buffer;	//transmit package
tCAN CANRX_buffer;	//receive package

volatile uint8_t rx_flag = 0;

/**********************************************************************************
	Interrupt Related Variables
*********************************************************************************/

//initialize tca write so internal pressure state machine starts correctly
//these 3 flags to indicate state within state of internal pressure read
//this state within state is required since a write to TCA and a write to MPL
//are required before MPL (pressure) can be read
uint8_t volatile TCA_WRITE = 1;
uint8_t volatile MPL_WRITE = 0;
uint8_t volatile MPL_READ = 0;

//History is used to select the index of the array of historical sample data
//Data is saved every time the entire set of sensors is recorded and is saved
//history increments from 0 to max (i.e. 0-9)
uint8_t volatile history = 0;
uint8_t volatile ep_history = 0;
uint16_t volatile adResult = 0;

//adCount used to determine odd numbered ADC reads and also contains info on 
//which sensor is being read. Odd ADC reads used to ensure no funny business with
//switching channels
uint8_t volatile adCount = 0;
uint8_t volatile adcDone = 0;

//indicates if pressure or temperature data triggers a critical system interrupt (i.e. CAN CI frame)
uint8_t volatile crit_error = 0;

//updated values from i2c status register upon interrupt
uint8_t volatile i2c_status = 0;

//Still need to determine if want to halt i2c action on ADC read
//If yes, use this flag
uint8_t volatile busy = 0;

//Indicates position in i2c read state machine - i.e. internal pressure sensor #2
uint8_t volatile state = 0;

/*****************************************************************************************
	Other state  machine / debugging variables
*****************************************************************************************/

//Debug flags to check i2c status
uint8_t write_done = 0;
uint8_t read_done = 0;

//Debug string for output
unsigned char string_out[25];
unsigned char i2c_status_error[30]= "\n\ri2c status not recognized\n\r";

//used to track if a number of consecutive unsuccessful i2c transmit
//Related Error handling still needs to be written
uint8_t nack_count = 0;

//Time stamp variable. To be implemented later. Incremented every 16.7ms timer
uint32_t run_time = 0;

//Array of bytes read from current i2c cycle - transferred after stop condition
uint8_t i2c_temp[8];	

//Counts number of bytes in i2c read/write cycle
uint8_t count=0;

/****************************************************************
	Data Logging Arrays
*****************************************************************/

//Temperature Sensor 1, U8
int16_t t1_raw[10];		//10*(raw 13 bit signed temperature value)
uint8_t t1_aux[10][5];	//temp status codes, time stamp

//Temperature Sensor 2, U9
int16_t t2_raw[10];		//10*(raw 13 bit signed temperature value)
uint8_t t2_aux[10][5];	//temp status codes, time stamp

//Temperature Sensor 3, U10
int16_t t3_raw[10];		//10*(raw 13 bit signed temperature value)
uint8_t t3_aux[10][5];	//temp status codes, time stamp

//Internal Pressure Sensor 1, U4
uint32_t ip1_raw[10];
uint8_t ip1_aux[10];

//Internal Pressure Sensor 2, U6
uint32_t ip2_raw[10];
uint8_t ip2_aux[10];

//Internal Pressure Sensor 3, U7
uint32_t ip3_raw[10];
uint8_t ip3_aux[10];

uint16_t ep1_raw[10];
uint16_t ep2_raw[10];
uint16_t ep3_raw[10];

ISR(TIMER1_COMPA_vect)
{
	//GENERATE START CONDITION IF I2C DISABLED
	if(~(TWCR0 & 0b00000100))
	{
		//timer increments state number and should toggle an LED every 1 second
		if(state < LAST_STATE)
		{
			state++;
			init_i2c();	
		}
		else
		{
			//LED_TOGGLE;
			state = 1;
			init_i2c();
		}
	}
	run_time++;
}

ISR(TIMER0_COMPA_vect)
{
	//start ADC conversion
	ADCSRA |= (1<<ADSC);
	//LED_TOGGLE;
}

ISR(ADC_vect)
{
	if(adCount != 5)
	{
		adCount++;
	}
	else
	{
		adCount = 0;
	}
	
	if(adCount % 2){

		adResult = ADCL;
		adResult |= (ADCH << 8);

		if(adCount < 5)
		{
			if(adCount == 1)
			{
				ep1_raw[ep_history] = adResult;
				//AREF, Right Adjusted, ADC6
				ADMUX = 0b00000110;
			}
			if(adCount == 3)
			{
				ep2_raw[ep_history] = adResult;
				//AREF, Right Adjusted, ADC7
				ADMUX = 0b00000111;
			}
		}
		if(adCount == 5)
		{
			ep3_raw[ep_history] = adResult;

			//AREF, Right Adjusted, ADC0
			ADMUX = 0b00000000;

			//if on last ADC cycle, increment "time row"
			if(ep_history<9)
			{
				ep_history++;
			}
			else
			{
				ep_history = 0;
				LED_TOGGLE;
			}
		}
	adcDone = 1;
	}
	
}

ISR(USART0_RX_vect)
{
	if(!(UCSR0A & (1<<7)))//if data in serial buffer
	{
		//get serial data
		CANTX_buffer.data[0] = USART_Receive();

		//transmit usart_char over canbus
		mcp2515_send_message(&CANTX_buffer);
	}
}

ISR(INT0_vect)
{
	mcp2515_get_message(&CANRX_buffer);//get canbus message
	rx_flag = 1;  //set flag
}

ISR (TWI0_vect)	//twi interrupt
{
	//interrupt set i2c module changes states;
	i2c_status = 0b11111100 & TWSR0;		//read TWSR register and mask prescaler bits
	//uint8_t i2c_flag = 1;
}

//STATE is incremented when the TWEN is disabled
int main(void)
{
	sys_config();
	init_ADC_timer();
	init_ADC();
	initUART();
	INTERRUPT_init();
	USART_Init(103);//103 sets baud rate at 9600
	SPI_masterInit();

	//DEBUG STRINGING
	sprintf(string_out, "UART INIT. %d\n\r", i2c_status);
	sendString(string_out);
	//END DEBUG STRINGING

	//init_int_press();	//blocking code - i2c transmit only
	//init_state_timer(); //will increment state iff i2c disabled
	//verify_int_press();
	//verify_ext_press();

   while (1) 
    {
		//i2c_FSM();
		
		//Ext Press. ADC Triggered by timer 0 interrupt
		get_ext_press();	//loads data array if 2nd adc conversion on given channel is complete (Channels: 6,7,0)

		//if data received on CAN...
		if(rx_flag){
			ATOMIC_BLOCK(ATOMIC_FORCEON){//disables interrupts
				//[FOR DEBUGGING]transmits received frame over uart.
				USART_CAN_TX(CANRX_buffer);
				
				//matches received ID. 
				switch (CANRX_buffer.id)
				{
					case RR_Press_ID: 

							USART_CAN_TX(Request_Response_SIB_Pressure);
							//send over uart
							mcp2515_send_message(&Request_Response_SIB_Pressure);
							//send over can
							break;

					case RR_Temp_ID:

							USART_CAN_TX(Request_Response_SIB_Temp);
							//send over uart
							mcp2515_send_message(&Request_Response_SIB_Temp);
							//send over can
							break;
					default: break;
				}
				rx_flag = 0;//clear receive flag
			}//end ATOMIC_BLOCK
		}

		if(crit_error)
		{
			ATOMIC_BLOCK(ATOMIC_FORCEON)
			{
				switch(crit_error)
				{
					case LEAK: 
						mcp2515_send_message(&CI_Leak);
						break;
					case DEPTH:
						mcp2515_send_message(&CI_Depth);
						break;
					case GENERAL:
						mcp2515_send_message(&CI_SIB_General);
						break;
				}
			}
		}
    }
}

//tCAN Request_Response_SIB_Pressure 	= {.id = 392, .header.rtr = 0, .header.length = 6};
//tCAN Request_Response_SIB_Temp 		= {.id = 400, .header.rtr = 0, .header.length = 3};

void init_state_timer(void)
{
	OCR1AH = 0b00000101;
	OCR1AL = 0b00010110;	//top value for timer = 1301
		
	TCCR1B = 0b00001101;		//set prescaler to 1024, CTC operation
	TIMSK1 = 0b00000010;		//enable compare match a interrupt
}

void i2c_FSM(void)
{
	
	//this function makes sure all variables are defined properly for each state
	// then calls i2c read/write dependant on state

	uint8_t r_address;		//i2c address of device to be read depending on state
	uint8_t t_address;		//i2c address of device to be written depending on state
	uint8_t readCycle;		//number of data bytes to read [not including address]
	uint8_t writeCycle;		//number of data bytes to write [not including address]
	int16_t t1;
	
	if(TWCR0 & 0b10000000)	//if TWINT high 
	{
		switch(state)
		{
			case 1: 
					if(TCA_WRITE)
					{	
						//TCA9544A	
						r_address = 0b11100001;
						t_address = 0b11100000;
						writeCycle = 1;

						//set control register config as data to send
						//Select TCA Channel 0
						i2c_temp[0] = 0b00000100; //Channel 2 select: 0b00000110;
						i2c_write(t_address, writeCycle);
					}
					else if(MPL_WRITE)
					{
						//Internal Pressure 1
						t_address = 0b11000000;
						writeCycle = 1;

						//Set pressure data register 1 address as byte to send
						i2c_temp[0] = 0x01;	
						i2c_write(t_address, writeCycle);
					}
					else if (MPL_READ)
					{
						//Read pressure data, 3 bytes
						readCycle = 3;
						r_address = 0b11000001; 
						i2c_read(r_address, readCycle);
						
						//TWI must be disabled after stop condition here if stop generated
						//disabling TWI will trigger next state
						if(TWCR0 & 0b00010000)
						{
							CLEAR_TWEN;
							MPL_READ = 0;
							TCA_WRITE = 1;
						}
					}		
					
					break;
			//Internal Pressure 2
			case 2:
					if(TCA_WRITE)
					{
						//TCA9544A
						r_address = 0b11100001;
						t_address = 0b11100000;
						writeCycle = 1;

						//set control register config as data to send
						//Select TCA Channel 1
						i2c_temp[0] = 0b00000101;
						i2c_write(t_address, writeCycle);
					}
					else if(MPL_WRITE)
					{
						//Internal Pressure 1
						t_address = 0b11000000;
						writeCycle = 1;

						//Set pressure data register 1 address as byte to send
						i2c_temp[0] = 0x01;
						i2c_write(t_address, writeCycle);
					}
					else if (MPL_READ)
					{
						//Read pressure data, 3 bytes
						readCycle = 3;
						r_address = 0b11000001;
						i2c_read(r_address, readCycle);
						
						//TWI must be disabled after stop condition here if stop generated
						//disabling TWI will trigger next state
						if(TWCR0 & 0b00010000)
						{
							CLEAR_TWEN;
							MPL_READ = 0;
							TCA_WRITE = 1;
						}
					}
					
					break;
			//Internal Pressure 3
			case 3:
					if(TCA_WRITE)
					{
						//TCA9544A
						r_address = 0b11100001;
						t_address = 0b11100000;
						writeCycle = 1;

						//set control register config as data to send
						//Select TCA Channel 2
						i2c_temp[0] = 0b00000110;
						i2c_write(t_address, writeCycle);
					}
					else if(MPL_WRITE)
					{
						//Internal Pressure 1
						t_address = 0b11000000;
						writeCycle = 1;

						//Set pressure data register 1 address as byte to send
						i2c_temp[0] = 0x01;
						i2c_write(t_address, writeCycle);
					}
					else if (MPL_READ)
					{
						//Read pressure data, 3 bytes
						readCycle = 3;
						r_address = 0b11000001;
						i2c_read(r_address, readCycle);
						
						//TWI must be disabled after stop condition here if stop generated
						//disabling TWI will trigger next state
						if(TWCR0 & 0b00010000)
						{
							CLEAR_TWEN;
							MPL_READ = 0;
							TCA_WRITE = 1;
						}
					}
					
					break;
			//Temperature 1
			case 4: r_address = 0b10010001;
					t_address = 0b10010000;
					readCycle = 2;
					//writeCycle =
					i2c_read(r_address,readCycle);
					break;
			//Temperature 2
			case 5: //r_address = ;
					//t_address = ;
					//readCycle =
					//writeCycle =
					break;
			//Temperature 3
			case 6: //r_address = ;
					//taddress = ;
					//readCycle =
					//writeCycle =
					break;
		}
	}
}

void init_int_press(void)	//state 0 for duration of this function
{
	
	uint8_t cycle;
	init_i2c();
	i2c_temp[0] = 0b00000100;
	cycle = 1;
	while(!write_done)
	{
		sprintf(string_out, "check for i2c int %d\n\r", i2c_status);
		sendString(string_out);
		//select channel 0
		if(TWCR0 & 0b10000000)
		{
			sprintf(string_out, "entering write mod. %d\n\r", i2c_status);
			sendString(string_out);
			i2c_write(TCA_W_ADD, cycle);
		}
	}
	sprintf(string_out, "TCA WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);
	write_done = 0;

	init_i2c();
	i2c_temp[0] = 0x26;
	i2c_temp[1] = 0x01;
	cycle = 2;
	while(!write_done)
	{
		//write register address, activate sensor
		if(TWCR0 & 0b10000000)
		{
			i2c_write(MPL_W_ADD, cycle);
		}

	}
	sprintf(string_out, "MPL WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);
	write_done = 0;

	init_i2c();
	i2c_temp[0] = 0b00000101;
	while(!write_done)
	{
		//select channel 1
		if(TWCR0 & 0b10000000)
		{
			i2c_write(TCA_W_ADD, 1);
		}
	}
	write_done = 0;
	sprintf(string_out, "TCA WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);

	init_i2c();
	i2c_temp[0] = 0x26;
	i2c_temp[1] = 0x01;
	while(!write_done)
	{
		//write register address, activate sensor
		if(TWCR0 & 0b10000000)
		{
			i2c_write(MPL_W_ADD, 2);
		}
	}
	sprintf(string_out, "MPL WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);
	write_done = 0;

	init_i2c();
	i2c_temp[0] = 0b00000110;
	while(!write_done)
	{
		//sprintf(string_out, "check for i2c int %d\n\r", i2c_status);
		//sendString(string_out);

		//select channel 2
		if(TWCR0 & 0b10000000)
		{
			//sprintf(string_out, "entering write mod. %d\n\r", i2c_status);
			//sendString(string_out);

			i2c_write(TCA_W_ADD, 1);
		}
	}
	write_done = 0;

	init_i2c();
	i2c_temp[0] = 0x26;
	i2c_temp[1] = 0x01;
	while(!write_done)
	{
		//write register address, activate sensor
		if(TWCR0 & 0b10000000)
		{
			i2c_write(MPL_W_ADD, 2);
		}
	}
	//LED_ON;
	//string_out = "Pressure Sensors Init'd";
	//sendString(string_out);

}

void load_i2c_data(void)	//loads temporary i2c data into formatted data arrays
{
	uint16_t utemp = 0;
	uint8_t temp = 0;

	switch(state)
	{
		count = 0;
		/*case 4:
				t1_aux[history][0] = i2c_temp[0]>>3;
				t1_aux[history][1] = i2c_temp[1]
				temp = temp <<8;
				temp |= i2c_temp[1];
				*(t1_aux + history) = (0b00000111 & temp);
				t1_raw[history] = temp >>3;
			break;
		case 5:
				temp = i2c_temp[0];
				temp = utemp <<8;
				temp |= i2c_temp[1];
				t2_aux[history] = (0b00000111 & temp);
				t2_raw[history] = temp >>3;
			break;
		case 6:
				temp = i2c_temp[0];
				temp = temp <<8;
				temp |= i2c_temp[1];
				t3_aux[history] = (0b00000111 & temp);
				t3_raw[history] = temp >>3;
			break;*/
		//need to add logic to make leading bits 1 if bit 19 is 1
		case 1:
				utemp = i2c_temp[0];
				utemp = utemp <<8;
				utemp |= i2c_temp[1];
				utemp = utemp<<4;
				utemp |= i2c_temp[2];
				ip1_raw[history] = utemp;
			break;

		case 2:
				utemp = i2c_temp[0];
				utemp = utemp <<8;
				utemp |= i2c_temp[1];
				utemp = utemp<<4;
				utemp |= i2c_temp[2];
				ip2_raw[history] = utemp;
			break;
		case 3:
				utemp = i2c_temp[0];
				utemp = utemp <<8;
				utemp |= i2c_temp[1];
				utemp = utemp<<4;
				utemp |= i2c_temp[2];
				ip3_raw[history] = utemp;
				//load can temperature frame at this time
			break;
		default:
			break;
	}
}

void verify_int_press(void)	//Debugging function
{
	//wont start until state incremented by timer
	//then will read first sensor
	state = 1;
	while(state==1)
	{
		i2c_FSM();
	}
	sprintf(string_out, "int_p verified1/3: %d\n\r", ip1_raw[0]);
	sendString(string_out);
	sprintf(string_out, "int_p verified2/3: %d\n\r", ip1_raw[1]);
	sendString(string_out);
	sprintf(string_out, "int_p verified3/3: %d\n\r", ip1_raw[2]);
	sendString(string_out);
}

//add clearing bits in TWCR
void i2c_write(uint8_t address, uint8_t cycle)
{
	//i2c_write takes data from i2c_FSM
	//uses value of volatile i2c status register to control i2c module accordingly
	//SETTING TWINT writes a 1 to the bit, which clears the interrupt flag and makes bit 0
	//SETTING TWINT or clearing the interrupt flag should always be the last instruction, it makes hardware execute
	//TWSTO = Two wire interface stop condition
	//TWSTA = Two wire interface start condition

	switch(i2c_status)
	{
		//start condition transmitted,
		//clear interrupt flag, write address
		case START:

			//DEBUG STRINGING
			sprintf(string_out, "status START_W %d\n\r", state);
			sendString(string_out);
			//END DEBUG STRINGING
			count = 0;
			TWDR0 = address;
			CLEAR_TWSTO;
			CLEAR_TWSTA;
			SET_TWINT;

			break;

		//if repeated start condition generated
		//clear interrupt flag, write address
		case REP_START:

			//DEBUG STRINGING
			sprintf(string_out, "REP_START_W %d\n\r", state);
			sendString(string_out);
			//END DEBUG STRINGING
			count = 0;
			TWDR0 = address;
			CLEAR_TWSTA;
			CLEAR_TWSTO;
			SET_TWINT;

			break;

		//address+write/read transmitted, ack received
		//clear interrupt flag, write first data byte
		case T_ADD_ACK:

			sprintf(string_out, "T_ADD_ACK %d\n\r", state);
			sendString(string_out);
			nack_count = 0;
			TWDR0 = i2c_temp[count];
			CLEAR_TWSTA;
			CLEAR_TWSTO;
			SET_TWINT;
			break;

		//address+write transmitted, nack received -> resend address
		//if nack received 10+ times, send stop condition
		case T_ADD_NACK:

			sprintf(string_out, "T_ADD_NACK %d\n\r", state);
			sendString(string_out);
			nack_count++;
			if (nack_count > 10)
			{
				SET_TWSTO;
				CLEAR_TWSTA;
				SET_TWINT;
			}
			else
			{
				TWDR0 = address;
				CLEAR_TWSTA;
				CLEAR_TWSTO;
				SET_TWINT;
			}
			break;

		//data transmitted, ack received -> send data if not at end of cycle
		case T_ACK:

			nack_count = 0;
			if(count < cycle)
			{
				TWDR0 = i2c_temp[count];
				count++;
				CLEAR_TWSTA;
				CLEAR_TWSTO;
				//DEBUG STRINGING
				sprintf(string_out, "transmit_ack %d\n\r", state);
				sendString(string_out);
				//END DEBUG STRINGING
				SET_TWINT;
			}
			else
			{
				//if reached the last byte in the write cycle,
				//reset cycle & increment state to read next device
				count = 0;
				if((state <4) && (state !=0))	//if is internal pressure AND IN state machine loop
				{
					//TCA address indicates i2c mux channel select will be sent
					//generates stop condition so sensor can be addressed next
					if(address == TCA_W_ADD)
					{
						CLEAR_TWSTA;
						SET_TWSTO;
						MPL_WRITE = 1;
						MPL_READ = 0;
						TCA_WRITE = 0;
						write_done = 1;
						SET_TWINT;
					}
					//if internal pressure state, but address is for pressure sensor
					//generate repeated start so the device can be read
					else
					{
						SET_TWSTA;
						CLEAR_TWSTO;
			 			MPL_READ = 1;
						MPL_WRITE=0;
						write_done = 1;
						SET_TWINT;
					}
				}
				else
				{
					CLEAR_TWSTA;
					SET_TWSTO;
					write_done = 1;
					SET_TWINT;
					sprintf(string_out, "transmitted. state %d\n\r", state);
					sendString(string_out);
					CLEAR_TWEN;
				}
			}
			break;

		//data transmitted, nack received -> resend address
		//if nack received 10+ times, send stop condition
		case T_NACK:

			nack_count++;
			sprintf(string_out, "T_NACK state %d\n\r", state);
			sendString(string_out);
			if (nack_count > 10)
			{
				SET_TWSTO;
				CLEAR_TWSTA;
				SET_TWINT;
			}
			else
			{
				TWDR0 = i2c_temp[count];
				CLEAR_TWSTA;
				CLEAR_TWSTO;
				SET_TWINT;
			}
			break;

			default:
			sendString(i2c_status_error);
			break;
	}
	
}

void i2c_read(uint8_t address, uint8_t cycle)
{
	//i2c_read takes data from i2c_FSM
	//uses value of volatile i2c status register to control i2c module accordingly

	int16_t t1;

	switch(i2c_status)
	{
		//start condition transmitted
		//clear interrupt flag, sends address
		case START:

		//DEBUG STRINGING
		sprintf(string_out, "status start %d\n\r", i2c_status);
		sendString(string_out);
		//END DEBUG STRINGING

		count = 0;
		TWDR0 = address;
		CLEAR_TWSTO;
		CLEAR_TWSTA;
		SET_TWINT;
		break;

		//repeated start transmitted
		//clear interrupt flag, sends address
		case REP_START:
		
		//DEBUG STRINGING
		sprintf(string_out, "status rep_start %d\n\r", i2c_status);
		sendString(string_out);
		//END DEBUG STRINGING

		count = 0;
		TWDR0 = address;
		CLEAR_TWSTO;
		CLEAR_TWSTA;
		SET_TWINT;
		break;

		//address+read transmitted, ack received -> send ack or nack
		case R_ADD_ACK:

		//DEBUG STRINGING
		sprintf(string_out, "status addr ack %d\n\r", i2c_status);
		sendString(string_out);
		//END DEBUG STRINGING

		nack_count = 0;
		if(busy)
		{
			CLEAR_TWEA;			//clear enable acknowledge
			CLEAR_TWSTO;
			CLEAR_TWSTA;
			SET_TWINT;			//clear interrupt flag, sends nack
		}
		else
		{
			SET_TWEA;			//enable acknowledge
			CLEAR_TWSTO;
			CLEAR_TWSTA;
			SET_TWINT;			//clear interrupt flag, sends ack
		}
		break;

		//address+write transmitted, nack received -> generate stop condition/error function or repeated start
		case R_ADD_NACK:

		//DEBUG STRINGING
		sprintf(string_out, "status addr nack %d\n\r", i2c_status);
		sendString(string_out);
		//END DEBUG STRINGING

		nack_count++;
		if (nack_count > 10)
		{
			CLEAR_TWSTA;
			SET_TWSTO;
			SET_TWINT;
		}
		else
		{
			SET_TWSTA;
			CLEAR_TWSTO;
			SET_TWINT;
		}
		
		//error(i2c_status, address);	write error function to indicate which sensor not communicating
		break;

		//data byte received, ack has been returned -> read data byte if within read cycle
		case R_ACK:

		//DEBUG STRINGING
		sprintf(string_out, "status data ack %d\n\r", i2c_status);
		sendString(string_out);
		//END DEBUG STRINGING
		
		if(count + 1 < cycle)
		{
			i2c_temp[count] = TWDR0;
			count++;
			CLEAR_TWSTA;
			CLEAR_TWSTO;
			SET_TWEA;
			SET_TWINT;
		}
		else
		{
			//if reached the last byte in the read cycle - read, reset cycle, send nack
			i2c_temp[count] = TWDR0;
			count = 0;
			CLEAR_TWSTA;
			CLEAR_TWSTO;
			CLEAR_TWEA;
			SET_TWINT;

			//DEBUG STRINGING
			sprintf(string_out, "last byte rec. %d\n\r", i2c_status);
			sendString(string_out);
			//END DEBUG STRINGING
			
		}
		break;

		//data byte received, nack returned -> stop condition
		//transfer received information
		case R_NACK:
			//DEBUG STRINGING
			_delay_ms(10);
			sprintf(string_out, "status STOP COND. %d\n\r", i2c_status);
			sendString(string_out);
			_delay_ms(10);
			//END DEBUG STRINGING

			//disable global interrupts to format 16 bit int
			MCUCR &=0b11111110;
			load_i2c_data();
			MCUCR |=0b00000001;
			//enable global interrupts

			count = 0;
			CLEAR_TWSTA;
			SET_TWSTO;
			SET_TWINT;
			CLEAR_TWEN;
			//DISABLE I2C

			//DEBUG STRINGING
			sprintf(string_out, "stop generated %d\n\r", i2c_status);
			sendString(string_out);
			_delay_ms(20);
			sprintf(string_out, "T: .625C * %d\n\r", t1);
			sendString(string_out);
			_delay_ms(20);
			//END DEBUG STRINGING

		break;

		case ARB:							//arbitration lost	- N/A
		break;
	}
}

void init_i2c(void)
{
	SET_TWEN;
	SET_TWIE;
	TWSR0 |= 0b00000001;	//prescaler = 4
	TWBR0 = 3;				//twbr = 3
	SET_TWSTA;
	SET_TWINT;
	SET_TWSTA;
	//SCL freq = 400k
}

void get_ext_press(void)
{
	char extPress[20];
	char extPressState[20];
	char sensorNum[13];
	uint8_t sensor;
	uint16_t data_out;

	if(adcDone)
	{
		ATOMIC_BLOCK(ATOMIC_FORCEON)
		{
			if(adCount == 1)
			{
				sensor = 1;
				data_out = ep1_raw[ep_history];
			}
			if(adCount == 3)
			{
				sensor = 2;
				data_out = ep2_raw[ep_history];
			}
			if(adCount == 5)
			{
				sensor = 3;
				data_out = ep3_raw[ep_history];
				
				ext_press_CAN();
			}

			sprintf(extPressState, "Sample Count-%u", ep_history);
			sendString(extPressState);
			sprintf(sensorNum, "- Sensor - %u", sensor);
			sendString(sensorNum);
			sprintf(extPress, ", Value = %u\n\r", data_out );
			sendString(extPress);
			//_delay_ms(20);
		}

		adcDone = 0;
	}
}

void ext_press_CAN(void)
{
	uint8_t diff1, diff2, diff3;
	uint16_t p_variation;
	uint8_t p1_dc = 0;
	uint8_t p2_dc = 0;
	uint8_t p3_dc = 0;					//indicates pressure sensor discontinuity - jumped to different depth
	uint8_t num_sensors;								//indicates number of valid sensors incorporated in pressure output
	uint8_t errors = 0;									//number of discontinuities
	uint16_t ep1;
	uint16_t ep2;
	uint16_t ep3;								//current external pressure values
	uint16_t median;
	uint16_t min;
	uint16_t max;

	ep1 = ep1_raw[history];
	ep2 = ep2_raw[history];
	ep3 = ep3_raw[history];

	//check pressure 1 continuity
	if(ep1_raw[history] > ep1_raw[history-1])
	{
		diff1 = ep1-ep1_raw[history-1];
	}
	else
	{
		diff1 = ep1_raw[history-1]-ep1;
	}

	//check pressure 2 continuity
	if(ep2_raw[history] > ep2_raw[history-1])
	{
		diff2 = ep2-ep2_raw[history-1];
	}
	else
	{
		diff2 = ep2_raw[history-1]-ep2;
	}

	//check pressure 3 continuity
	if(ep3_raw[history] > ep3_raw[history-1])
	{
		diff3 = ep3-ep3_raw[history-1];
	}
	else
	{
		diff3 = ep3_raw[history-1]-ep3;
	}

	if(diff1 > 50 || diff2 > 50 || diff3 > 50)
	{
		crit_error = DEPTH;
		if(diff1>50)
		{
			p1_dc = 1;
		}
		if(diff2>50)
		{
			p2_dc = 1;
		}
		if(diff3>50)
		{
			p3_dc = 1;
		}
	}
	errors = p1_dc+p2_dc+p3_dc;

	switch (errors)
	{
		case 0:
			num_sensors = 3;
			if((ep1 == ep2) && (ep2 == ep3))
			{
				median = ep3;
			}
			else if ((ep1 == ep2) || (ep1 == ep3))
			{
				median = ep1;
			}
			else if (ep2 == ep3)
			{
				median = ep2;
			}
			else
			{
				median = getMedian_of3(ep1,ep2,ep3);
			}
			break;
		case 1:
			num_sensors = 2;
			if ((ep1 == ep2) || (ep1 == ep3))
			{
				median = ep1;
			}
			else if (ep2 == ep3)
			{
				median = ep2;
			}
			break;
		case 2:
			num_sensors=1;

			if(!p1_dc)
			{median = ep1;}

			else if(!p2_dc)
			{median = ep2;}

			else 
			{median = ep3;}
			break;
		case 3:
			median = 0xFF;
			num_sensors = 0;
			break;

	}

	//NEED TO MODIFY FRAME SO VARIATION IS 5 BITS
	max = getMax(ep1,ep2,ep3);
	min =  getMin(ep1,ep2,ep3);
	p_variation = 100*(max-min)/min;
	
	Request_Response_SIB_Pressure.data[0] = 0;
	Request_Response_SIB_Pressure.data[1] = 0;
	Request_Response_SIB_Pressure.data[2] = 0;
	Request_Response_SIB_Pressure.data[3]  = (median<<2) | (num_sensors);
	Request_Response_SIB_Pressure.data[4]  = (median>>4) | (p_variation<<4);

	sprintf(string_out, "RR_SIB_Press = %u\n\r", Request_Response_SIB_Pressure.data);
	sendString(string_out);

	//sprintf(string_out, "%u\n\r", Request_Response_SIB_Pressure.data[4] );
	//sendString(string_out);
}
