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
	-determine ADC input range to map to for each input/resistor value
		-adjust response frame to tegra request
	-setup over/under current critical interrupt
		-setup error lights accordingly
	
=============================================================================*/

/******************************************************************************
	Prebuilt Includes|
******************************************************************************/
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
#define Rx0ID  0x000
#define Rx1ID  0x000
#define Rx2ID  0x001
#define Rx3ID  0x001
#define Rx4ID  0x001
#define Rx5ID  0x001
/******************************************************************************/

/******************************************************************************
	Headerfiles|
		Order is important!
******************************************************************************/
#include "headers/global.h"			//general define header pulled from net
#include "headers/defines.h"		//Pin name definitions
#include "headers/functions.h"		//general functions
#include "headers/spi_ry.h"			//SPI protocol implementation
#include "headers/mcp2515_ry_def.h"	//MCP2515 register and bit definitions
#include "headers/mcp2515_ry.h"		//MCP2515 functions
#include "headers/can_frames.h"		//CAN frames in tCAN struct format
#include "headers/usart_ry.h"		//serial communication with PC


		
/******************************************************************************
	Gloval Variables|
******************************************************************************/
tCAN CANTX_buffer;	//transmit package
tCAN CANRX_buffer;	//receive package

volatile uint8_t rx_flag = 0; //receive interupt flag
volatile uint8_t Rx_frame_state = start_byte; //receive state machine counter

uint16_t voltage_sense[3];
uint8_t adc_select = 0;

/******************************************************************************
	start of main()|
******************************************************************************/
int main(void)
{
  	//initialization functions
	GPIO_init();
	INTERRUPT_init();
	ADC_init();
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
	
	
	ADCSRA |= (1<<ADSC); //start adc sample
	
	while(1)
    {
		//if data received on CAN...
		if(rx_flag){
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){//disables interrupts
				//[FOR DEBUGGING]transimts received frame over uart.
				USART_CAN_TX(CANRX_buffer);
				TOGGLE(LED4);
				
				//matches received ID.  if current request, returns
				//	current data
				//if more cases are required, will convert to a switch-case
				if(CANRX_buffer.id == Read_Request_Backplane_Current.id){
					USART_CAN_TX(Request_Response_Backplane_Current);
						//send over uart
					mcp2515_send_message(&Request_Response_Weapon_status);
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
		
			voltage_sens = {ADC7, ADC6, ADC0} 
			which corresponds to
			voltage_sens = {P6V_SENSE, P24V_SENS, P5V_SENSE} 
	*/
	voltage_sense[adc_select] = ADCH;
	
	adc_select++;
	if(adc_select > 2){//resets count at 3 and stores values in CAN frame
		adc_select = 0;
		for(uint8_t j = 0; j < 3; j++){
		Request_Response_Backplane_Current.data[j] = voltage_sense[j];
		}
	}
	
	//select which adc to sample from
	switch(adc_select){
		case 0 : ADMUX &= 0b11110000; //set ADC0
			break;	
		case 1 : ADMUX &= 0b11110110; //set ADC6
		break;
		case 2 : ADMUX &= 0b11110111; //set ADC7
		break;
		default : ADMUX &= 0b11110000; //set ADC0
		break;
	}
	ADCSRA |= (1<<ADSC); //start adc sample
}



/******************************************************************************
	USART Receive interrupt|
	
	"UART Confined CAN FRAME"(UCCF) defined in excel file
		~/"RoboSub 17 CAN Frames Rev.4.xlsx"
******************************************************************************/
ISR(USART0_RX_vect)
{
	uint8_t receive_buff = USART_Receive();
	
	//select which adc to sample from
	switch(Rx_frame_state){
		case s_RxStart : //start byte
		if (receive_buff == start_byte){
			Rx_frame_state = s_RxIDH;
		}
		break;
		case s_RxIDH : //frameID High
			CANTX_buffer.id |= receive_buff <<3;
			Rx_frame_state = s_RxIDL;
		break;
		case s_RxIDL : //frameID Low, rtr, & length = 0bXXXYZZZZ
			CANTX_buffer.id |= (receive_buff >>5);
			CANTX_buffer.header.rtr =  ((receive_buff >>4) & 0x01);
			CANTX_buffer.header.length = (receive_buff & 0x0F);
			if(CANTX_buffer.header.rtr){
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			} else {
				Rx_frame_state = s_Rxdata1;
			}
		break;
		case s_Rxdata1 : //data1
			CANTX_buffer.data[0] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata2;
			}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata2 ://data2
			CANTX_buffer.data[1] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata3;
				}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata3 ://data3
			CANTX_buffer.data[2] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata4;
				}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata4 ://data4
			CANTX_buffer.data[3] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata5;
				}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata5 ://data5
			CANTX_buffer.data[4] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata6;
				}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata6 ://data6
			CANTX_buffer.data[5] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata7;
				}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata7 ://data7
			CANTX_buffer.data[6] = receive_buff;
			if(Rx_frame_state < CANTX_buffer.header.length){
				Rx_frame_state = s_Rxdata8;
				}else{
				mcp2515_send_message(&CANTX_buffer);
				receive_buff = 0;
				Rx_frame_state = s_RxIDH;
			}
		break;
		case s_Rxdata8 ://data8
			CANTX_buffer.data[7] = receive_buff;
			mcp2515_send_message(&CANTX_buffer);
			receive_buff = 0;
			Rx_frame_state = s_RxIDH;
		break;
		default : Rx_frame_state = s_RxIDH;
		break;
	}
}
