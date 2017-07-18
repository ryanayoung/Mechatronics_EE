/*
 * SIB_1.c
 * Created: 6/3/2017 9:56:57 AM
 * Author : Scott Szafranski
 */ 

 /******************************************
 TO DO:
	-implement time stamps?
	-implement discontinuity checks
		*MCP Pressure Delta Interrupt
		*LM92 Interrupt? / Manual check
	-implement Temperature threshold checks
		*LM92 Interrupt for crit temp
		*Set LM92 warning temp
	-implement watchdog timer
	-implement external pressure error checking
********************************************/

#define F_CPU 16000000UL
#include "headers/defines.h"		//Pin name definitions

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
#include "headers/uart.h"
#include "headers/functions.h"		//general functions
#include "headers/spi_ry.h"			//SPI protocol implementation
#include "headers/mcp2515_ry_def.h"	//MCP2515 register and bit definitions
#include "headers/mcp2515_ry.h"		//MCP2515 functions
#include "headers/can_frames.h"
#include "headers/usart_ry.h"		//serial communication with PC
#include "headers/ADC.h"

/******************************************************************************
	CANBUS ID definition|
******************************************************************************/
//RxID is your device ID that you allow messages to receive
uint16_t RxID = 0x03;	//SIB

/******************************************************************************
	CANBUS Related variabes
*******************************************************************************/
tCAN CANTX_buffer;	//transmit package
tCAN CANRX_buffer;	//receive package

volatile uint8_t rx_flag = 0;
uint8_t crit_depth = 0;
uint8_t crit_leak = 0;
uint8_t crit_general = 0;

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
uint8_t volatile adc_Cycle_Done = 0;

//indicates if pressure or temperature data triggers a critical system interrupt (i.e. CAN CI frame)
uint8_t volatile crit_error = 0;

//updated values from i2c status register upon interrupt
uint8_t volatile i2c_status = 0;

//Still need to determine if want to halt i2c action on ADC read
//If yes, use this flag
uint8_t volatile busy = 0;

//Indicates position in i2c read state machine - i.e. internal pressure sensor #2
uint8_t volatile state = 0;
uint8_t volatile state_ready = 0;
uint8_t volatile end_state = 0;

/*****************************************************************************************
	Other state  machine / debugging variables
*****************************************************************************************/

//Debug flags to check i2c status
uint8_t write_done = 0;
uint8_t read_done = 0;

//Debug string for output

char i2c_status_error[35]= "\n\ri2c status not recognized\n\r";

//used to track if a number of consecutive unsuccessful i2c transmit
//Related Error handling still needs to be written
uint8_t nack_count = 0;

//Time stamp variable. To be implemented later. Incremented every 16.7ms timer
//uint32_t run_time = 0;

//Array of bytes read from current i2c cycle - transferred after stop condition
uint8_t i2c_temp[8];	

//Counts number of bytes in i2c read/write cycle
uint8_t count=0;

/****************************************************************
	Data Logging Arrays
*****************************************************************/

//Temperature Sensor 1, U8
int16_t t1_raw[10];		//10*(raw 13 bit signed temperature value)
//uint16_t t1_aux[10];	//temp status codes, time stamp

//Temperature Sensor 2, U9
int16_t t2_raw[10];		//10*(raw 13 bit signed temperature value)
//uint16_t t1_aux[10];	//temp status codes, time stamp

//Temperature Sensor 3, U10
int16_t t3_raw[10];		//10*(raw 13 bit signed temperature value)
//uint16_t t3_aux[10];	//temp status codes, time stamp

//Internal Pressure Sensor 1, U4
uint32_t ip1_raw[10];
//uint16_t ip1_aux[10];

//Internal Pressure Sensor 2, U6
uint32_t ip2_raw[10];
//uint16_t ip2_aux[10];

//Internal Pressure Sensor 3, U7
uint32_t ip3_raw[10];
//uint16_t ip3_aux[10];

uint16_t ep1_raw[10];
uint16_t ep2_raw[10];
uint16_t ep3_raw[10];

volatile uint8_t i2c_errors[10][7]; //10 time slots, 
//6 states, using 7 item array in order to use state numbers directly
//bit 8= T_ADD NACK, bit 7 = T_NACK
//bit 6 = R_ADD NACK, bit 5 = R_NACK
//bit 4 = MPL, bit 3 = TCA, bit 2 = LM92

ISR(TIMER1_COMPA_vect)
{
	//GENERATE START CONDITION IF I2C DISABLED
	if(end_state)
	{
		sprintf(string_out, "state %d, ++\n\r", state);
		sendString(string_out);
		
		//timer increments state number and should toggle an LED every 1 second
		if(state < LAST_STATE)
		{
			state++;
			MPL_READ = 0;
			MPL_WRITE = 0;
			TCA_WRITE = 1;
			i2c_errors[history][state] = 0;
			init_i2c();	
		}
		else
		{
			//LED_TOGGLE;
			state = 1;
			MPL_READ = 0;
			MPL_WRITE = 0;
			TCA_WRITE = 1;
			i2c_errors[history][state] = 0;
			init_i2c();
		}
		end_state = 0;
		state_ready = 1;
	}
	//run_time++;
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
	
	if(adCount % 2)	//if odd #'d conversion
	{
		adResult = ADCL;
		adResult |= (ADCH << 8);

		if(adCount < 5)
		{
			if(adCount == 1)
			{
					//if on first ADC cycle, increment "time row"
					if(ep_history<9)
					{
						ep_history++;
					}
					else
					{
						ep_history = 0;
						LED_TOGGLE;
					}
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

			adc_Cycle_Done = 1;
		}
	
	}
	
}

ISR(INT0_vect)
{
	mcp2515_get_message(&CANRX_buffer);//get canbus message
	rx_flag = 1;  //set flag
}

ISR (TWI0_vect)	//i2c interrupt
{
	//interrupt set i2c module changes states;
	i2c_status = 0b11111100 & TWSR0;		//read TWSR register and mask prescaler bits
	//uint8_t i2c_flag = 1;
}

//STATE is incremented when the ~17ms timer triggers, iff state_end = 1
int main(void)
{
	sys_config();
	
	init_ADC_timer();
	init_ADC();
	initUART();
	INTERRUPT_init();
	USART_Init(103);	//103 sets baud rate at 9600
	SPI_masterInit();
	init_int_press();	//blocking code - i2c transmit only, disables TWI

	init_state_timer(); //will increment state iff i2c disabled
	state_ready = 1;
	end_state = 1;

	//MCP2515 initialization
	if(mcp2515_init(CANSPEED_500))
	{//[TODO]these need to be updated to RAW serial messages
		USART_Transmit_TX("Can Init SUCCESS!");
	}else
	{
		USART_Transmit_TX("Can Init FAILURE!");
	}
	_delay_ms(100);

	while (1) 
    {
		int_p1();
		int_p2();
		update_ep();
		CAN_HANDLE();
		int_p3();
		int_t1();
		update_ep();
		CAN_HANDLE();
		int_t2();
		int_t3();
		update_ep();
		debug_can_tx();
		CAN_HANDLE();
		//CAN_TX_INT(crit_depth,crit_leak,crit_general);
    }
}

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
	uint8_t error;
	
	if(TWCR0 & 0b10000000)	//if TWINT high 
	{
		switch(state)
		{
			case IP1: 
					if(TCA_WRITE)
					{	
						//TCA9544A	
						r_address = 0b11100001;
						t_address = 0b11100000;
						writeCycle = 1;

						//set control register config as data to send
						//Select TCA Channel 0
						i2c_temp[0] = 0b00000100; //Channel 2 select: 0b00000110;
						error = i2c_write(t_address, writeCycle);
						error_TCA(error);
						if(write_done==1)
						{
							write_done = 0;
							init_i2c();
						}
					}
					else if(MPL_WRITE)
					{
						//Internal Pressure 1
						t_address = 0b11000000;
						writeCycle = 1;

						//Set pressure data register 1 address as byte to send
						i2c_temp[0] = 0x01;							
						error = i2c_write(t_address, writeCycle);
						error_MPLW(error);
					}
					else if (MPL_READ)
					{
						//Read pressure data, 3 bytes
						readCycle = 3;
						r_address = 0b11000001; 
						error = i2c_read(r_address, readCycle);
						error_MPLR(error);
						
						//TWI must be disabled after stop condition here if stop generated
						//disabling TWI will trigger next state
						if(read_done == 1)
						{
							read_done = 0;
							debug_state();
							CLEAR_TWEN;
							end_state = 1;
						}
					}			
					break;


			//Internal Pressure 2
			case IP2:
					if(TCA_WRITE)
					{
						//TCA9544A
						r_address = 0b11100001;
						t_address = 0b11100000;
						writeCycle = 1;

						//set control register config as data to send
						//Select TCA Channel 1
						i2c_temp[0] = 0b00000101;
						error = i2c_write(t_address, writeCycle);
						error_TCA(error);
						if(write_done==1)
						{
							write_done = 0;
							init_i2c();
						}
					}
					else if(MPL_WRITE)
					{
						//Internal Pressure 1
						t_address = 0b11000000;
						writeCycle = 1;

						//Set pressure data register 1 address as byte to send
						i2c_temp[0] = 0x01;
						error = i2c_write(t_address, writeCycle);
						error_MPLW(error);
					}
					else if (MPL_READ)
					{
						//Read pressure data, 3 bytes
						readCycle = 3;
						r_address = 0b11000001;
						error = i2c_read(r_address, readCycle);
						error_MPLR(error);
						
						//TWI must be disabled after stop condition here if stop generated
						//disabling TWI will trigger next state
						if(read_done==1)
						{
							read_done = 0;
							debug_state();
							CLEAR_TWEN;
							end_state = 1;
						}
					}
					break;


			//Internal Pressure 3
			case IP3:
					if(TCA_WRITE)
					{
						//TCA9544A
						r_address = 0b11100001;
						t_address = 0b11100000;
						writeCycle = 1;

						//set control register config as data to send
						//Select TCA Channel 2
						i2c_temp[0] = 0b00000110;
						error = i2c_write(t_address, writeCycle);
						error_TCA(error);
						if(write_done==1)
						{
							write_done = 0;
							init_i2c();
						}
					}
					else if(MPL_WRITE)
					{
						//Internal Pressure 1
						t_address = 0b11000000;
						writeCycle = 1;

						//Set pressure data register 1 address as byte to send
						i2c_temp[0] = 0x01;
						error = i2c_write(t_address, writeCycle);
						error_MPLW(error);
					}
					else if (MPL_READ)
					{
						//Read pressure data, 3 bytes
						readCycle = 3;
						r_address = 0b11000001;
						error = i2c_read(r_address, readCycle);
						error_MPLR(error);
						
						//TWI must be disabled after stop condition here if stop generated
						//disabling TWI will trigger next state
						if(read_done ==1)
						{
							read_done =0;
							ip_CAN();
							debug_state();
							CLEAR_TWEN;
							end_state = 1;
						}
					}
					break;


			//Temperature 1, U8
			case T1: r_address = 0b10010001;
					t_address = 0b10010000;
					readCycle = 2;
					//writeCycle =
					error = i2c_read(r_address,readCycle);
					error_LM92(error);
					if(read_done ==1)
					{
						read_done = 0;
						debug_state();
						CLEAR_TWEN;
						end_state = 1;
					}
					//else if read_done == 2, error
					break;


			//Temperature 2, U9
			case T2: r_address = 0b10010011;
					//t_address = ;
					readCycle =2;
					//writeCycle =
					error = i2c_read(r_address,readCycle);
					error_LM92(error);
					if(read_done ==1)
					{
						read_done = 0;
						debug_state();
						CLEAR_TWEN;
						end_state = 1;
					}
					break;


			//Temperature 3, U10
			case T3: r_address = 0b10010101;
					//taddress = ;
					readCycle =2;
					//writeCycle =
					error = i2c_read(r_address,readCycle);
					error_LM92(error);
					if(read_done == 1)
					{
						read_done =0;
						debug_state();
						CLEAR_TWEN;
						end_state = 1;
						it_CAN();
					}
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
	while(!(write_done==1))
	{
		//select channel 0
		if(TWCR0 & 0b10000000)	//if TWI Interrupt
		{
			i2c_write(TCA_W_ADD, cycle);
		}
	}


	sprintf(string_out, "TCA WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);
	write_done = 0;


	init_i2c();
	//Select control Register
	i2c_temp[0] = 0x26;
	//128x Oversampling, Active Mode
	i2c_temp[1] = 0x39;

	cycle = 2;
	while(!(write_done==1))
	{
		if(TWCR0 & 0b10000000)	//If TWI Interrupt
		{
			i2c_write(MPL_W_ADD, cycle);
		}
	}


	sprintf(string_out, "MPL WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);
	write_done = 0;


	init_i2c();
	i2c_temp[0] = 0b00000101;
	while(!(write_done==1))
	{
		//select channel 1
		if(TWCR0 & 0b10000000)	//If TWI Interrupt
		{
			i2c_write(TCA_W_ADD, 1);
		}
	}
	write_done = 0;


	sprintf(string_out, "TCA WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);

	init_i2c();
	//Select control Register
	i2c_temp[0] = 0x26;
	//128x Oversampling, Active Mode
	i2c_temp[1] = 0x39;

	cycle = 2;
	while(!(write_done==1))
	{
		if(TWCR0 & 0b10000000)	//if TWI Interrupt
		{
			i2c_write(MPL_W_ADD, cycle);
		}
	}


	sprintf(string_out, "MPL WRITE DONE %d\n\r", i2c_status);
	sendString(string_out);
	write_done = 0;


	init_i2c();
	i2c_temp[0] = 0b00000110;
	while(!(write_done==1))
	{
		//select channel 2
		if(TWCR0 & 0b10000000)
		{
			i2c_write(TCA_W_ADD, 1);
		}
	}


	write_done = 0;
	sprintf(string_out, "TCA CH2 SELECTED %d\n\r", i2c_status);
	sendString(string_out);


	init_i2c();
	//Select control Register
	i2c_temp[0] = 0x26;
	//128x Oversampling, Active Mode
	i2c_temp[1] = 0x39;

	cycle = 2;
	while(!(write_done==1))
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
	//LED_ON;
	//string_out = "Pressure Sensors Init'd";
	//sendString(string_out);*/
}

void load_i2c_data(void)	//loads temporary i2c data into formatted data arrays
{
	uint32_t utemp = 0;
	uint16_t temp = 0;

	switch(state)
	{
		case T1:
			if(i2c_errors[history][state])
			{	
				t1_raw[history] = 0xFFFF;
			}
			else
			{
				temp = i2c_temp[0];
				temp = temp <<8;
				temp |= i2c_temp[1];
				//if leading one (negative)
				//actually it will never be negative
				if (temp & 0b10000000)
				{
					temp = ((temp >>3) | 0b11100000);
				}
				else
				{
					temp = temp >> 3;
				}
				t1_raw[history] = temp;
			}
			break;

		case T2:
			if(i2c_errors[history][state])
			{
				t2_raw[history] = 0xFFFF;
			}
			else
			{
				temp = i2c_temp[0];
				temp = temp <<8;
				temp |= i2c_temp[1];
				//if leading one (negative)
				if (temp & 0b10000000)
				{
					temp = ((temp >>3) | 0b11100000);
				}
				else
				{
					temp = temp >> 3;
				}
				t2_raw[history] = temp;
			}
			break;

		case T3:
			if(i2c_errors[history][state])
			{
				t3_raw[history] = 0xFFFF;
			}
			else
			{
				temp = i2c_temp[0];
				temp = temp <<8;
				temp |= i2c_temp[1];
				//if leading one (negative)
				if (temp & 0b10000000)
				{
					temp = ((temp >>3) | 0b11100000);
				}
				else
				{
					temp = temp >> 3;
				}
				t3_raw[history] = temp;
			}
			break;

		case IP1:
			if(i2c_errors[history][state])
			{
				ip1_raw[history] = 0xFFFFFFFF;
			}
			else
			{
				//increment to next value in data set array
				if (history == 9)
				{
					history = 0;
				}
				else
				{
					history++;
				}
				utemp = i2c_temp[0];
				utemp = utemp <<8;
				utemp |= i2c_temp[1];
				utemp = utemp<<4;
				utemp |= i2c_temp[2];
				ip1_raw[history] = utemp;
			}
			break;

		case IP2:
			if(i2c_errors[history][state])
			{
				ip2_raw[history] = 0xFFFFFFFF;
			}
			else
			{
				utemp = i2c_temp[0];
				utemp = utemp <<8;
				utemp |= i2c_temp[1];
				utemp = utemp<<4;
				utemp |= i2c_temp[2];
				ip2_raw[history] = utemp;
			}
			break;

		case IP3:
			if(i2c_errors[history][state])
			{
				ip3_raw[history] = 0xFFFFFFFF;
			}
			else
			{
				utemp = i2c_temp[0];
				utemp = utemp <<8;
				utemp |= i2c_temp[1];
				utemp = utemp<<4;
				utemp |= i2c_temp[2];
				ip3_raw[history] = utemp;
			}
			break;
		default:
			break;
	}
}

uint8_t i2c_write(uint8_t address, uint8_t cycle)
{
	//i2c_write takes data from i2c_FSM
	//uses value of volatile i2c status register to control i2c module accordingly
	//SETTING TWINT writes a 1 to the bit, which clears the interrupt flag and makes bit 0
	//SETTING TWINT or clearing the interrupt flag should always be the last instruction, it makes hardware execute
	//TWSTO = Two wire interface stop condition
	//TWSTA = Two wire interface start condition
	if(TWCR0 & (1<<TWEN)){
	switch(i2c_status)
	{
		//start condition transmitted,
		//clear interrupt flag, write address
		case START:

			//DEBUG STRINGING
			//sprintf(string_out, "status START_W %d\n\r", state);
			//sendString(string_out);
			//END DEBUG STRINGING
			count = 0;
			TWDR0 = address;
			I2C_NEXT;
			break;

		//if repeated start condition generated
		//clear interrupt flag, write address
		case REP_START:

			//DEBUG STRINGING
			/*sprintf(string_out, "REP_START_W %d\n\r", state);
			sendString(string_out);*/
			//END DEBUG STRINGING
			count = 0;
			TWDR0 = address;
			I2C_NEXT;
			break;

		//address+write/read transmitted, ack received
		//clear interrupt flag, write first data byte
		case T_ADD_ACK:

			//sprintf(string_out, "T_ADD_ACK %d\n\r", state);
			//sendString(string_out);
			nack_count = 0;
			TWDR0 = i2c_temp[count];
			I2C_NEXT;
			break;

		//address+write transmitted, nack received -> resend address
		//if nack received 10+ times, send stop condition
		case T_ADD_NACK:

			/*sprintf(string_out, "T_ADD_NACK %d\n\r", state);
			sendString(string_out);*/
			nack_count++;
			if (nack_count > 10)
			{
				//CLEAR_TWEN;
				//sprintf(string_out, "T_ADD_NACKx10, %d", state);
				//sendString(string_out);
				write_done = 2;	//signifies error
				I2C_STOP;
				return T_ADD_NACK;
			}
			else
			{
				TWDR0 = address;
				I2C_NEXT;
			}
			break;

		//data transmitted, ack received -> send data if not at end of cycle
		case T_ACK:

			nack_count = 0;
			if(count + 1< cycle)
			{
				//DEBUG STRINGING
				//sprintf(string_out, "transmit_ack %d\n\r", state);
				//sendString(string_out);
				//END DEBUG STRINGING
				count++;
				TWDR0 = i2c_temp[count];
				I2C_NEXT;
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
						MPL_WRITE = 1;
						MPL_READ = 0;
						TCA_WRITE = 0;
						write_done = 1;
						I2C_STOP;
					}
					//if internal pressure state, but address is for pressure sensor
					//generate repeated start so the device can be read
					else
					{
			 			MPL_READ = 1;
			 			MPL_WRITE=0;
			 			write_done = 1;
						I2C_START;
					}
				}
				else
				{
					write_done = 1;
					I2C_STOP;
					/*sprintf(string_out, "transmitted. state %d\n\r", state);
					sendString(string_out);*/
					//CLEAR_TWEN;
				}
			}
			break;

		//data transmitted, nack received -> resend address
		//if nack received 10+ times, send stop condition
		case T_NACK:

			nack_count++;
			/*sprintf(string_out, "T_NACK state %d\n\r", state);
			sendString(string_out);*/
			if (nack_count > 10)
			{
				sprintf(string_out, "T_NACKx10, %d", state);
				sendString(string_out);
				write_done = 2;
				I2C_STOP;
				return T_NACK;
			}
			else
			{
				TWDR0 = i2c_temp[count];
				I2C_NEXT;
			}
			break;

			default:
			break;
	}
	}
	return 0;	
}

uint8_t i2c_read(uint8_t address, uint8_t cycle)
{
	//i2c_read takes data from i2c_FSM
	//uses value of volatile i2c status register to control i2c module accordingly
	if (TWCR0 & (1<<TWEN))
	{
		switch(i2c_status)
		{
			//start condition transmitted
			//clear interrupt flag, sends address
			case START:
				count = 0;
				TWDR0 = address;
				I2C_NEXT;
				break;

			//repeated start transmitted
			//clear interrupt flag, sends address
			case REP_START:
				count = 0;
				TWDR0 = address;
				I2C_NEXT;
				break;

			//address+read transmitted, ack received -> send ack or nack
			case R_ADD_ACK:
				nack_count = 0;
				if(busy)
				{
					I2C_NEXT;		//clear interrupt flag, sends nack
				}
				else
				{
					I2C_ACK;		//clear interrupt flag, sends ack
				}
				break;

			//address+write transmitted, nack received -> generate stop condition/error function or repeated start
			case R_ADD_NACK:

				nack_count++;
				if (nack_count > 10)
				{
					read_done = 1;
					I2C_STOP;
					return R_ADD_ACK;
				}
				else
				{
					I2C_START;
				}
			break;

			//data byte received, ack has been returned -> read data byte if within read cycle
			case R_ACK:

				if(count + 1 < cycle)
				{
					i2c_temp[count] = TWDR0;
					count++;
					I2C_ACK;
				}
				else
				{
					//if reached the last byte in the read cycle - read, reset cycle, send nack
					i2c_temp[count] = TWDR0;
					count = 0;
					I2C_NEXT;
				}
			break;

			//data byte received, nack returned -> stop condition
			//transfer received information
			case R_NACK:
				count = 0;
				ATOMIC_BLOCK(ATOMIC_FORCEON){load_i2c_data();}
				read_done = 1;
				I2C_STOP;
				break;

			case ARB:							//arbitration lost	- N/A
			break;
			default:
			break;
		}
	}
	return 0;
}

void init_i2c(void)
{
	TWSR0 |= 0b00000001;	//prescaler = 16, for 0x01, prescaler =4, scl =400kHz
	TWBR0 = 3;				//twbr = 3
	I2C_START;
	//SCL freq = 400k
}

void debug_state(void)
{
	return;
	/*switch(state)
	{
		case 1:
				sprintf(string_out, "IP read, state: %d\n\r", state);
				sendString(string_out);

				break;
		case 2:
				sprintf(string_out, "IP read, state: %d\n\r", state);
				sendString(string_out);
				break;

		case 3:
				sprintf(string_out, "IP read, state: %d\n\r", state);
				sendString(string_out);

				sprintf(string_out, "IP1: %l\n\r", ip1_raw[history]);
				sendString(string_out);
				sprintf(string_out, "IP2: %l\n\r", ip2_raw[history]);
				sendString(string_out);
				sprintf(string_out, "IP3: %l\n\r", ip3_raw[history]);
				sendString(string_out);
				break;
				
		case 4:
				sprintf(string_out, "T read, state: %d\n\r", state);
				sendString(string_out);
				break;

		case 5:
				sprintf(string_out, "T read, state: %d\n\r", state);
				sendString(string_out);
				break;

		case 6:
				sprintf(string_out, "T read, state: %d\n\r", state);
				sendString(string_out);

				sprintf(string_out, "T1: %d\n\r", t1_raw[history]);
				sendString(string_out);
				sprintf(string_out, "T2: %d\n\r", t2_raw[history]);
				sendString(string_out);
				sprintf(string_out, "T3: %d\n\r", t3_raw[history]);
				sendString(string_out);
				break;

	}*/
}

void int_p1(void)
{
		if(state_ready && (state == IP1))
		{
			state_ready = 0;
			while (state == IP1)
			{
				i2c_FSM();
			}
		}	
}

void int_p2(void)
{
		if(state_ready && state == IP2)
		{
			state_ready = 0;
			while (state == IP2)
			{
				i2c_FSM();
			}
		}
}

void int_p3(void)
{
		if(state_ready && state == IP3)
		{
			state_ready = 0;
			while (state == IP3)
			{
				i2c_FSM();
			}
		}
}

void int_t1(void)
{
		if(state_ready && state == T1)
		{
			state_ready = 0;
			while (state == T1)
			{
				i2c_FSM();
			}
		}
}

void int_t2(void)
{
		if(state_ready && state == T2)
		{
			state_ready = 0;
			while (state == T2)
			{
				i2c_FSM();
			}
		}
}

void int_t3(void)
{
		if(state_ready && state == T3)
		{
			state_ready = 0;
			while (state == T3)
			{
				i2c_FSM();
			}
		}
}

void ip_CAN(void)
{	
	ATOMIC_BLOCK(ATOMIC_FORCEON){

	Request_Response_SIB_Pressure.data[4] = ip1_raw[history];
	Request_Response_SIB_Pressure.data[5] = ip1_raw[history] >>8;
	Request_Response_SIB_Pressure.data[6] = ip1_raw[history] >>16 | (ip2_raw[history] << 4);
	Request_Response_SIB_Pressure.data[7] = ip2_raw[history] >>4;

	Request_Response_SIB_Temp.data[0] = ip2_raw[history] >> 12;
	Request_Response_SIB_Temp.data[1] = ip3_raw[history];
	Request_Response_SIB_Temp.data[2] = ip3_raw[history]>>8;
	Request_Response_SIB_Temp.data[3] = ip3_raw[history]>>16;
	//forgot sensor 3, going to leave for now since Jared would have to rewrite code
 	}
}

void ep_CAN(void)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON){
		Request_Response_SIB_Pressure.data[0] = ep1_raw[history];
		Request_Response_SIB_Pressure.data[1] = (ep1_raw[history] >>8) | (ep2_raw[history] <<2);
		Request_Response_SIB_Pressure.data[2] = (ep2_raw[history] >>6) | (ep3_raw[history] << 4);
		Request_Response_SIB_Pressure.data[3] = ep3_raw[history] >>4;
	}
}

void it_CAN(void)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON){
		Request_Response_SIB_Temp.data[3] = t1_raw[history]<<4;
		Request_Response_SIB_Temp.data[4] = t1_raw[history]>>4;
		Request_Response_SIB_Temp.data[5] = t2_raw[history];
		Request_Response_SIB_Temp.data[6] = t2_raw[history]>>8 | t3_raw[history]<<4;
		Request_Response_SIB_Temp.data[7] = t3_raw[history] >> 4;
	}
}

void update_ep(void)
{
	if(adc_Cycle_Done)
	{
		//IF ADC ISR LAST STATE/CHANNEL
		//ext_press_CAN();
		sprintf(string_out, "EP1: %d\n\r", ep1_raw[history]);
		sendString(string_out);
		sprintf(string_out, "EP1: %d\n\r", ep1_raw[history]);
		sendString(string_out);
		sprintf(string_out, "EP1: %d\n\r", ep1_raw[history]);
		sendString(string_out);
		adc_Cycle_Done = 0;
	}
}

void CAN_HANDLE(void)
{
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


					//SHOULD include this frame too since data is lumped together
					//mcp2515_send_message(&Request_Response_SIB_Temp);
					break;

				case RR_Temp_ID:
					//this case may become obsolete
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
}

void CAN_TX_INT(uint8_t cd,uint8_t cl,uint8_t cg)
{
		/*if(crit_depth || crit_leak || crit_general)
		{
			ATOMIC_BLOCK(ATOMIC_FORCEON)
			{
				if(crit_leak)
				{		
					mcp2515_send_message(&CI_Leak);
					crit_leak = 0;
				}
				if(crit_depth)
				{
					mcp2515_send_message(&CI_Depth);
					crit_depth = 0;
				}
				if(crit_general)
				{
					mcp2515_send_message(&CI_SIB_General);
					crit_general = 0;
				}
			}
		}*/
}

void debug_can_tx(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		/*CI_SIB_General.data[1] = t1_raw[history];
		CI_SIB_General.data[2] = (t1_raw[history]>>8) | (t2_raw[history]<<4);
		CI_SIB_General.data[3] = (t2_raw[history]>>4);
		CI_SIB_General.data[4] = (t3_raw[history]);
		CI_SIB_General.data[5] = (t3_raw[history]>>8);*/
		USART_CAN_TX(Request_Response_SIB_Temp);
		mcp2515_send_message(&Request_Response_SIB_Temp);
		mcp2515_send_message(&Request_Response_SIB_Pressure);
		USART_CAN_TX(Request_Response_SIB_Pressure);
		//mcp2515_send_message(&CI_ESC1);
		//mcp2515_send_message(&CI_Kill_Switch);
		//USART_CAN_TX(Request_Response_SIB_Temp);
	}
}

void error_TCA(uint8_t type)
{
	switch(type)
	{
		case T_ADD_NACK:
			i2c_errors[history][state] |= (1<<7) | (1<<2);
			end_state = 1;
			break;
		case T_NACK:
			i2c_errors[history][state] |= (1<<6) | (1<<2);
			end_state = 1;
			break;
		default: break;
	}
}

void error_MPLW(uint8_t type)
{
	switch(type)
	{
		case T_ADD_NACK:
			i2c_errors[history][state] |= (1<<7) | (1<<3);
			end_state = 1;
			break;
		case T_NACK:
			i2c_errors[history][state] |= (1<<6) | (1<<3);
			end_state = 1;
			break;
		default: break;
	}
}

void error_MPLR(uint8_t type)
{
	switch(type)
	{
		case R_ADD_NACK:
			i2c_errors[history][state] = (1<<5) | (1<<3);
			end_state =1;
			break;
		case R_NACK:
			i2c_errors[history][state] = (1<<4) | (1<<3);
			end_state = 1;
			break;
		default: break;
	}
}

void error_LM92(uint8_t type)
{
	switch(type)
	{
		case R_ADD_NACK:
			i2c_errors[history][state] |= (1<<5) | (1<<1);
			end_state = 1;
			break;
		case R_NACK:
			i2c_errors[history][state] |= (1<<4) | (1<<1);
			end_state = 1;
			break;
		default: break;
	}
}