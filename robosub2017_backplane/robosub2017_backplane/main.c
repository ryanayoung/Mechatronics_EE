/*****************************************************************************
	main.c

	Description:
		Backplane for SDSU Mechatronics 2017 Robosub Perseverance
		
	Created:	06/13/2017
	Author:		Ryan Young
	Email:		RyanaYoung81@gmail.com
******************************************************************************/

/*=============================================================================
				[TODO]
				
	-change UART to interrupt driven code
	-determine ADC input rant to map to for each input/resistor value
		-adjust response frame to tegra request
	-setup over/under current critical interrupt
		-setup error lights accordingly
	
	-possibly add acceptance of all can messages intended for tegra into
		buffer1 and relay over serial & trigger error leds
	-setup backup serial2can interface if tegra can board doesn't work.
		-create FIFO buffer and do zero message interpretting.
=============================================================================*/

#define F_CPU 16000000UL // 16MHz clock from the debug processor
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/atomic.h>


/******************************************************************************
	CANBUS ID definition|
		
******************************************************************************/
//RxID is your device ID that you allow messages to receive
uint16_t RxID = 0x001;  

//TxID is the target ID you're transmitting to
uint8_t TxID = 0x020;	//M
//uint8_t TxID = 0x10;	//S
/******************************************************************************/


#include "headers/global.h"			//general define header pulled from net
#include "headers/defines.h"		//Pin name definitions
#include "headers/functions.h"		//general functions
#include "headers/spi_ry.h"			//SPI protocol implementation
#include "headers/usart_ry.h"		//serial communication with PC
#include "headers/mcp2515_ry_def.h"	//MCP2515 register and bit definitions
#include "headers/mcp2515_ry.h"		//MCP2515 functions

tCAN CANTX_buffer;	//transmit package
tCAN CANRX_buffer;	//receive package

volatile uint8_t rx_flag = 0;

uint16_t voltage_sense[4];
uint8_t adc_select = 0;

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
	{//[TODO]these need to be updated to RAW serial messages
		USART_Transmit_TX("Can Init SUCCESS!");
	}else
	{
		USART_Transmit_TX("Can Init FAILURE!");
	}
	USART_Transmit(10);//New Line
	USART_Transmit(13);//Carriage return
	
	//setup the transmit frame
	CANTX_buffer.id = TxID;			//set target device ID
	CANTX_buffer.header.rtr = 0;		//no remote transmit(i.e. request info)
	UARTTX_buCANTX_bufferffer.header.length = 1;	//single byte(could be up to 8)
	
	while(1)
    {
		
	
		//if data received on CAN...
		if(rx_flag){
			ATOMIC_BLOCK(ATOMIC_FORCEON){//disables interrupts
				//[FOR DEBUGGING]transimts received frame over uart.
				USART_CAN_TX(CANRX_buffer);
				
				//matches received ID.  if current request, returns
				//	current data
				//if more cases are required, will convert to a switch-case
				if(CANRX_buffer.id == Read_Request_Backplane_Current.id){
					USART_CAN_TX(Request_Response_Backplane_Current);
						//send over uart
					mcp2515_send_message(&Request_Response_Backplane_Current);
						//send over can
				}
				rx_flag = 0;//clear receive flag
			}//end ATOMIC_BLOCK
		}
		
    }
}

/******************************************************************************
	CAN RECEIVE interrupt on pin PD2|
******************************************************************************/
ISR(INT0_vect)
{
	mcp2515_get_message(&CANRX_buffer);//get canbus message
	rx_flag = 1;  //set flag
}

/******************************************************************************* 
	 ADC conversion complete ISR|
*******************************************************************************/ 
ISR(ADC_vect) 
{ 
	/*
		store ADC value remapped to 0-5.0Volts
		this allows them to fit into byte size readings instead of 10bit.
		hopefully we don't need the full 10 bits.
		should sit at 2.5V.  above 2.5V is + current, below 2.5 is 
		negative current.
		
			voltage_sens = {ADC0, ADC1, ADC6, ADC7} 
			which corresponds to
			voltage_sens = {P5V_SENSE, P20V_SENSE, P24V_SENS, P6V_SENSE} 
	*/
	voltage_sense[adc_select] = map(ADC, 0, 1023, 0, 50);
	
	
	adc_select++;
	if(adc_select > 3){//resets count at 4 and stores values in CAN frame
		adc_select = 0;
		Request_Response_Backplane_Current.data = voltage_sense;
	}
	
	//select which adc to sample from
	switch(adc_select){
		case 0 : ADMUX &= 0b11110000; //set ADC0
			break;	
		case 1 : ADMUX &= 0b11110001; //set ADC1
		break;
		case 2 : ADMUX &= 0b11110110; //set ADC6
		break;
		case 3 : ADMUX &= 0b11110111; //set ADC7
		break;
		default : ADMUX &= 0b11110000; //set ADC0
		break;
	}
	ADCSRA |= (1<<ADSC); //start adc sample
}



/******************************************************************************
	USART Receive interrupt|
	
	[TODO]
	Implement ring buffer
	update to receive a full can frame
		-define "start byte" as 0xEE
	"UART Confined CAN FRAME"(UCCF) defined in excel file
		~/"RoboSub 17 CAN Frames Rev.4.xlsx"
******************************************************************************/
ISR(USART0_RX_vect)
{
	if(!(UCSR0A & (1<<RXC)))//if data in serial buffer
	{
		//get serial data
		CANTX_buffer.data[0] = USART_Receive();

		//transmit usart_char over canbus
		mcp2515_send_message(&CANTX_buffer);
	}
}
