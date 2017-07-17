/******************************************************************************
	MAIN.c

	Description:
		Communicate between  microcontrollers over CAN-BUS protocol using
		the MCP2515 CAN transceiver chip.

	Created:	12/6/2016 8:08:43 AM
	Author:		Ryan Young
	RedID:		817447547
******************************************************************************/
//template for ATmega328p
#define F_CPU 16000000UL // 16MHz clock from the debug processor
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/atomic.h>


/******************************************************************************
	CANBUS ID definition|
		change values for different devices so that M can talk to S and
		visa versa
******************************************************************************/
//RxID is your device ID that you allow messages to receive
//uint8_t RxID = 0x10;  //M
//uint8_t RxID = 0x20;	//S
/*** currently no filter, receiving all messages***/


/******************************************************************************/


#include "headers/global.h"			//general define header pulled from net
#include "headers/defines.h"		//Pin name definitions
#include "headers/functions.h"		//general functions
#include "headers/spi_ry.h"			//SPI protocol implementation
#include "headers/usart_ry.h"		//serial communication with PC
#include "headers/mcp2515_ry_def.h"	//MCP2515 register and bit definitions
#include "headers/mcp2515_ry.h"		//MCP2515 functions
#include "headers/can_frames.h"



tCAN CANTX_buffer;	//transmit package
tCAN CANRX_buffer;		//receive package

volatile uint8_t rx_flag = 0;
volatile uint8_t Rx_frame_state = s_RxStart;

/******************************************************************************
	start of main()|
******************************************************************************/
int main(void)
{
	//initialization functions
	GPIO_init();
	INTERRUPT_init();
	USART_Init(103);//103 sets baud rate at 9600
	SPI_masterInit();
	
	
	//MCP2515 initialization
	if(mcp2515_init(CANSPEED_500))
	{
		USART_Transmit(0xFF);
	}else
	{
		USART_Transmit(0x00);
	}
	
	while (1)
	{
		
		//if rx_flag is set, that means there's a received message stored in
		//spi_char, so ATOMIC_BLOCK disabled interrupts, then transmits it
		//over uart.
		if(rx_flag){
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				USART_CAN_TX(CANRX_buffer);
				CANRX_buffer.id = 0;
				CANRX_buffer.header.rtr = 0;
				CANRX_buffer.header.length = 0;
				memset(CANRX_buffer.data, 0, sizeof(CANRX_buffer.data));
				rx_flag = 0;
				}
		}
	}
}/*****end of main()**********************************************************/

/******************************************************************************
	RECEIVE interrupt on pin PD2|
******************************************************************************/
ISR(INT0_vect)
{
	mcp2515_get_message(&CANRX_buffer);//get canbus message
	rx_flag = 1;  //set flag
}

ISR(USART_RX_vect)
{	
	uint8_t receive_buff = USART_Receive();
	
	switch(Rx_frame_state){
		case s_RxStart : //start byte
		if (receive_buff == start_byte){
			CANTX_buffer.id = 0;
			CANTX_buffer.header.rtr = 0;
			CANTX_buffer.header.length = 0;
			memset(CANTX_buffer.data, 0, sizeof(CANTX_buffer.data));
			Rx_frame_state = s_RxIDH;
		}
		break;
		case s_RxIDH : //frameID High
		CANTX_buffer.id |= (receive_buff << 3);
		Rx_frame_state = s_RxIDL;
		break;
		case s_RxIDL : //frameID Low, rtr, & length = 0bXXXYZZZZ
		CANTX_buffer.id |= (receive_buff >> 5);
		CANTX_buffer.header.rtr =  ((receive_buff >> 4) & 0x01);
		CANTX_buffer.header.length = (receive_buff & 0x0F);
		if(CANTX_buffer.header.rtr){
			USART_CAN_TX(CANTX_buffer);//[DEBUG]
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
			} else {
			Rx_frame_state = s_Rxdata1;
		}
		//USART_CAN_TX(CANTX_buffer);//[DEBUG]
		break;
		case s_Rxdata1 : //data1
		CANTX_buffer.data[0] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata2;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata2 ://data2
		CANTX_buffer.data[1] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata3;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata3 ://data3
		CANTX_buffer.data[2] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata4;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata4 ://data4
		CANTX_buffer.data[3] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata5;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata5 ://data5
		CANTX_buffer.data[4] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata6;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata6 ://data6
		CANTX_buffer.data[5] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata7;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata7 ://data7
		CANTX_buffer.data[6] = receive_buff;
		if(Rx_frame_state < CANTX_buffer.header.length){
			Rx_frame_state = s_Rxdata8;
			}else{
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		}
		break;
		case s_Rxdata8 ://data8
		CANTX_buffer.data[7] = receive_buff;
		mcp2515_send_message(&CANTX_buffer);
		receive_buff = 0;
		Rx_frame_state = s_RxStart;
		break;
		default : Rx_frame_state = s_RxStart;
		break;
	}
	
}