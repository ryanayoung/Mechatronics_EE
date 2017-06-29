/*****************************************************************************
	main.c

	Description:
		Backplane for SDSU Mechatronics 2017 Robosub Perseverance
		

	Created:	06/13/2017
	Author:		Ryan Young
	Email:		RyanaYoung81@gmail.com
******************************************************************************/

/*=============================================================================
				TODO
				
	-RxID needs to be updated, as well as masks edited
	-a lookup table of all of the id's.  
		-Actually, might want to make tCAN structs for each device
		and function.  Possible as it's own header to reuse in
		everyone's code.
=============================================================================*/

#define F_CPU 16000000UL // 16MHz clock from the debug processor
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>


/******************************************************************************
	CANBUS ID definition|
		
******************************************************************************/
//RxID is your device ID that you allow messages to receive
uint8_t RxID = 0x10;  //M
//uint8_t RxID = 0x20;	//S

//TxID is the target ID you're transmitting to
uint8_t TxID = 0x20;	//M
//uint8_t TxID = 0x10;	//S
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

/******************************************************************************
	start of main()|
******************************************************************************/
int main(void)
{
    //set output pins
	DDRD |= (1<<DDD3); //pin 1 - error LED 1
	DDRD |= (1<<DDD4); //pin 2 - error LED 2
	DDRD |= (1<<DDD5); //pin 9 - error LED 3
	DDRD |= (1<<DDD6); //pin 10 - error LED 4
	
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
	USART_Transmit(10);//New Line
	USART_Transmit(13);//Carriage return
	
	//setup the transmit frame
	usart_char.id = TxID;			//set target device ID
	usart_char.header.rtr = 0;		//no remote transmit(i.e. request info)
	usart_char.header.length = 1;	//single byte(could be up to 8)
	
	while(1)
    {
		if(!(UCSR0A & (1<<RXC0)))//if data in serial buffer
		{
			//get serial data
			usart_char.data[0] = USART_Receive();
			
			//transmit usart_char over canbus
			mcp2515_send_message(&usart_char);
		}
		
		PIND |= (1<<PIND3); //toggle LED
		_delay_ms(500);
		PIND |= (1<<PIND4); //toggle LED
		_delay_ms(500);
		PIND |= (1<<PIND5); //toggle LED
		_delay_ms(500);
		PIND |= (1<<PIND6); //toggle LED
		_delay_ms(500);
    }
}

/******************************************************************************
	RECEIVE interrupt on pin PD2|
******************************************************************************/
ISR(INT0_vect)
{
	mcp2515_get_message(&spi_char);//get canbus message
	USART_Transmit(spi_char.data[0]); //transmit message over uart
}
