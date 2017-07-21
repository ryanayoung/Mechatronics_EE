/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */


#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__PIC24E__)
    	#include <p24Exxxx.h>
    #elif defined (__PIC24F__)||defined (__PIC24FK__)
	#include <p24Fxxxx.h>
    #elif defined(__PIC24H__)
	#include <p24Hxxxx.h>
    #endif
#endif
#define _XTAL_FREQ 16000000UL
#define F_CPU 16000000UL
//#define SYS_FREQ 32000000L
#define FCY SYS_FREQ/2
#define UART_BAUD 9600L
#include <stdio.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <libpic30.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h" 
#include "pic_global.h"
#include "defines.h"
<<<<<<< HEAD
#include "mcp2515_ry_def.h"
#include "pic_spi_ry.h"         /* User funct/params, such as InitApp              */
#include "functions.h"
#include "usart.h"
=======
#include "pic_spi_ry.h"
#include "mcp2515_ry.h"
#include "mcp2515_ry_def.h"
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */

>>>>>>> 9a37bd4d1ab245d6b5fe4ad036046a60c25e007a
/******************************************************************************
	CANBUS ID definition|
		change values for different devices so that M can talk to S and
		visa versa
******************************************************************************/
//RxID is your device ID that you allow messages to receive
//uint8_t RxID = 0x10;  //M
uint8_t RxID = 0x20;	//S

//TxID is the target ID you're transmitting to
//uint8_t TxID = 0x20;	//M
uint8_t TxID = 0x10;	//S
/******************************************************************************/
#include "mcp2515_ry.h"


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
// 16MHz clock from the debug processor




/******************************************************************************
	start of main()|
******************************************************************************/
tCAN usart_char;	//transmit package
tCAN spi_char;		//receive package
int main(void)
{
  AD1PCFG = 0xFFFF;
		//initialization functions
	GPIO_init();
	INTERRUPT_init();
	//USART_Init(103);//103 sets baud rate at 9600
	SPI_masterInit();

	//MCP2515 initialization
	if(mcp2515_init(CANSPEED_500))
	{
		USART_Transmit(0xFF);
	}else
	{
		USART_Transmit(0x00);
	}

	//setup the transmit frame
	usart_char.id = TxID;			//set target device ID
	usart_char.header.rtr = 0;		//no remote transmit(i.e. request info)
	usart_char.header.length = 1;	//single byte(could be up to 8)

/*	while (1)
	{
		if(!(UCSR0A & (1<<RXC0)))//if data in serial buffer
		{
			//get serial data
			usart_char.data[0] = USART_Receive();

			//transmit usart_char over canbus
			mcp2515_send_message(&usart_char);
		}
	}*/
    return 0;
}/****end of main()**********************************************************/

/******************************************************************************
	RECEIVE interrupt on pin PD2|
******************************************************************************/
/*ISR(INT0_vect)
{
	mcp2515_get_message(&spi_char);//get canbus message
	USART_Transmit(spi_char.data[0]); //transmit message over uart
}*/
