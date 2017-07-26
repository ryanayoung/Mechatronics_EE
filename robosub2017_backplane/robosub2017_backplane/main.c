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
#define Rx2ID  0x000
#define Rx3ID  0x000
#define Rx4ID  0x000
#define Rx5ID  0x000
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

volatile uint8_t rx_flag = 0; //receive interrupt flag
volatile uint8_t uart_rx_flag = 0; //receive interrupt flag
volatile uint8_t Rx_frame_state = start_byte; //receive state machine counter
volatile uint8_t pinC_change_state;

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
	
	//store current button state
	pinC_change_state = PINC;
	
	//MCP2515 initialization
	if(mcp2515_init(CANSPEED_500)){
		USART_Transmit(0xFF);
	}else
	{
		USART_Transmit(0x00);
	}	
	
	//start first adc sample
	ADCSRA |= (1<<ADSC); 
	
	while(1)
    {
		if(uart_rx_flag){
			if(CANTX_buffer.id == Read_Request_Backplane_Current.id){
				USART_CAN_TX(Request_Response_Backplane_Current);
			}
			uart_rx_flag = 0;
		}
		
		//if data received on CAN...
		if(rx_flag){
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){//disables interrupts
				USART_CAN_TX(CANRX_buffer);
				TOGGLE(LED4);
				
				//matches received ID.  if current request, returns
				//	current data
				//if more cases are required, will convert to a switch-case
				if(CANRX_buffer.id == Read_Request_Backplane_Current.id){
					mcp2515_send_message(&Read_Request_Backplane_Current);
						//send over can
				}
				CANRX_buffer.id = 0;
				CANRX_buffer.header.rtr = 0;
				CANRX_buffer.header.length = 0;
				rx_flag = 0;//clear receive flag
			}//end ATOMIC_BLOCK
		}
    }
}

/******************************************************************************
	CAN RECEIVE interrupt on pin PD2|
******************************************************************************/
ISR(INT0_vect){
	mcp2515_get_message(&CANRX_buffer);//get canbus message
	rx_flag = 1;  //set flag
}

/******************************************************************************
	Pin Change interrupts|
		RUN_EN and KILLSWITCH
******************************************************************************/
ISR(PCINT1_vect){
	
	uint8_t pin_state_OLD = pinC_change_state;//save previous state
	pinC_change_state = PINC;//get new state
	
	//if RUN_EN has gone low
	if(!(pinC_change_state & (1<<PINC2))){
		USART_CAN_TX(CI_Auto);
	}
	
	//if KILLSWITCH state has changed
	if((pinC_change_state & (1<<PINC3)) != (pin_state_OLD & (1<<PINC3))){
		USART_CAN_TX(CI_Kill_Switch);
	}
}

/******************************************************************************* 
	 ADC conversion complete ISR|
*******************************************************************************/ 
ISR(ADC_vect){ 
	int8_t V6_sense;
	int8_t V24_sense;
	int8_t V5_sense;
	
	/*
			voltage_sens = {ADC7, ADC6, ADC0} 
			which corresponds to
			voltage_sens = {P6V_SENSE, P24V_SENS, P5V_SENSE} 
	*/
	voltage_sense[adc_select] = ADCL;
	voltage_sense[adc_select] |= (uint16_t)(ADCH<<8);
	
	V6_sense = map(voltage_sense[0],0,1023,-125,125);//map to +/-12.5Amps
	V24_sense = map(voltage_sense[1],0,1023,-125,125);
	V5_sense = map(voltage_sense[2],0,1023,-50,50);//map to +/-5Amps
	
	if(V6_sense > 70){
		CI_Backplane_Current.data[0] = 4;
		CI_Backplane_Current.data[1] = V6_sense;
		USART_CAN_TX(CI_Backplane_Current);
	}
	if(V24_sense > 70){
		CI_Backplane_Current.data[0] = 5;
		CI_Backplane_Current.data[1] = V24_sense;
		USART_CAN_TX(CI_Backplane_Current);
	}
	if(V5_sense > 45){
		CI_Backplane_Current.data[0] = 6;
		CI_Backplane_Current.data[1] = V5_sense;
		USART_CAN_TX(CI_Backplane_Current);
	}
	adc_select++;
	if(adc_select > 2){//resets count at 3 and stores values in CAN frame
		adc_select = 0;
		Request_Response_Backplane_Current.data[0] = V6_sense;
		Request_Response_Backplane_Current.data[1] = V24_sense;
		Request_Response_Backplane_Current.data[2] = V5_sense;
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
		~/"RoboSub 17 CAN Frames Rev.X.xlsx"
******************************************************************************/
ISR(USART0_RX_vect){
	uint8_t receive_buff = USART_Receive();
	
	//select which adc to sample from
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
			CANTX_buffer.id |= receive_buff <<3;
			Rx_frame_state = s_RxIDL;
		break;
		case s_RxIDL : //frameID Low, rtr, & length = 0bXXXYZZZZ
			CANTX_buffer.id |= (receive_buff >>5);
			CANTX_buffer.header.rtr =  ((receive_buff >>4) & 0x01);
			CANTX_buffer.header.length = (receive_buff & 0x0F);
			if(CANTX_buffer.header.rtr){
				mcp2515_send_message(&CANTX_buffer);
				uart_rx_flag = 1;
				receive_buff = 0;
				Rx_frame_state = s_RxStart;
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
				uart_rx_flag = 1;
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
				uart_rx_flag = 1;
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
				uart_rx_flag = 1;
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
				uart_rx_flag = 1;
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
				uart_rx_flag = 1;
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
				uart_rx_flag = 1;
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
				uart_rx_flag = 1;
				receive_buff = 0;
				Rx_frame_state = s_RxStart;
			}
		break;
		case s_Rxdata8 ://data8
			CANTX_buffer.data[7] = receive_buff;
			mcp2515_send_message(&CANTX_buffer);
			uart_rx_flag = 1;
			receive_buff = 0;
			Rx_frame_state = s_RxStart;
		break;
		default : Rx_frame_state = s_RxStart;
		break;
	}
}
