/******************************************************************
	SPI_master.c

	Description:


	Created:	12/3/2016 2:18:58 PM
	Author:		Ryan Young
	RedID:		817447547

	NOTES:

	
*******************************************************************/

//template for ATmega328p
#define F_CPU 16000000UL // 16MHz clock from the debug processor
#include <avr/io.h>
#include <util/delay.h>
#include "headers/global.h" //general define header pulled from net
#include "headers/defines.h"
#include "headers/spi_ry.h"
#include "headers/usart_ry.h"
#include "headers/mcp2515_ry_def.h"
#include "headers/mcp2515_ry.h"


int main(void)
{
	uint8_t usart_char;
	uint8_t spi_char;
	
	USART_Init(103);
	SPI_masterInit();
	
	USART_Transmit(13);
	
    while (1) 
    {
		if(!(UCSR0A & (1<<RXC0)))//if data in serial buffer
		{
			usart_char = USART_Receive();//get serial data
			SET_L(SS);//enable slave
			SPI_txrx(usart_char);
			spi_char = SPI_txrx(0x00);// send data over SPI and get data
			SET_H(SS);//disable slave
			USART_Transmit(spi_char);//send received data over serial
		}		
    }
}

