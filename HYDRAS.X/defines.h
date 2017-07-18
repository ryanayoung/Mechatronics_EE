/******************************************************************************
	defines.h

	Description:

	Created: 	12/3/2016 3:11:57 PM
	Author:		Ryan Young
	RedID:		817447547

******************************************************************************/
//gpio pins formatted for using global.h
#define SS   C,2
#define MOSI C,0
#define SCK B,3
#define INT B,14

//status light
#define LED2			B,0

#define RxIDLow	((RxID & 0x07) << 5)
#define RxIDHi	(uint8_t)(RxID >> 3)
