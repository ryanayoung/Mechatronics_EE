/******************************************************************************
	defines.h

	Description:
		
	Created: 	12/3/2016 3:11:57 PM
	Author:		Ryan Young
	RedID:		817447547

******************************************************************************/
//gpio pins formatted for using global.h 
#define SS B,2
#define MOSI B,3
#define SCK B,5
#define INT D,2

//status light
#define LED1			D,3
#define LED2			D,4
#define LED3			D,5
#define LED4			D,6

//ADC inputs
#define V24_pin			E,2
#define V6_pin			E,3
#define V5_pin			C,0
#define V20_pin			C,1

#define RxIDLow	((RxID & 0x07) << 5)
#define RxIDHi	(uint8_t)(RxID >> 3)

#define RxMaskLow 0xE0
#define RXMaskHi  0x00
