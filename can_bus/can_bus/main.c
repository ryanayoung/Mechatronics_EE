/******************************************************************************
	can_bus.c

	Description:
		Communicate between two microcontrollers over CAN-BUS protocol using
		the MCP2515 CAN transceiver chip.

	Created:	12/6/2016 8:08:43 AM
	Author:		Ryan Young
	RedID:		817447547
******************************************************************************/
//template for ATmega328p
#define F_CPU 16000000UL // 16MHz clock from the debug processor
#include <avr/io.h>
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

//TxID is the target ID you're transmitting to
//uint8_t TxID = 0x20;	//M
uint8_t TxID = 0x10;	//S
/******************************************************************************/


#include "headers/global.h"			//general define header pulled from net
#include "headers/defines.h"		//Pin name definitions
#include "headers/functions.h"		//general functions
#include "headers/spi_ry.h"			//SPI protocol implementation
#include "headers/usart_ry.h"		//serial communication with PC
#include "headers/mcp2515_ry_def.h"	//MCP2515 register and bit definitions
#include "headers/mcp2515_ry.h"		//MCP2515 functions

tCAN usart_char;	//transmit package
tCAN spi_char;		//receive package

volatile uint8_t rx_flag = 0;

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
		USART_Transmit_TX("Can Init SUCCESS!");
	}else
	{
		USART_Transmit_TX("Can Init FAILURE!");
	}
	
	
	//setup the transmit frame
	usart_char.id = TxID;			//set target device ID
	usart_char.header.rtr = 0;		//no remote transmit(i.e. request info)
	usart_char.header.length = 1;	//single byte(could be up to 8)
	
	while (1)
	{
		//if rx_flag is set, that means there's a received message stored in
		//spi_char, so ATOMIC_BLOCK disabled interrupts, then transmits it
		//over uart.
		if(rx_flag){
				ATOMIC_BLOCK(ATOMIC_FORCEON){
				USART_Transmit(spi_char.id >> 8); //CanID_High

				USART_Transmit(spi_char.id); //CandID_Low

				USART_Transmit(spi_char.header.rtr); //rtr

				USART_Transmit(spi_char.header.length); //length

				
				//read back all data received.
				if(!spi_char.header.rtr){
					for (uint8_t t = 0; t < spi_char.header.length;t++) {
						USART_Transmit(spi_char.data[t]); //data

					}
				}
				
				rx_flag = 0;
				}
		}
		/*
		if(!(UCSR0A & (1<<RXC0)))//if data in serial buffer
		{
			//get serial data
			usart_char.data[0] = USART_Receive();
			
			//transmit usart_char over canbus
			mcp2515_send_message(&usart_char);
		}
		*/
	}
}/*****end of main()**********************************************************/

/******************************************************************************
	RECEIVE interrupt on pin PD2|
******************************************************************************/
ISR(INT0_vect)
{
	mcp2515_get_message(&spi_char);//get canbus message
	rx_flag = 1;  //set flag
}
