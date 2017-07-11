/******************************************************************************
	defines.h

	Description:

	Created: 	12/3/2016 3:11:57 PM
	Author:		Ryan Young
	RedID:		817447547

******************************************************************************/
//gpio pins formatted for using global.h
#define SS   B,0
#define MOSI B,3
#define SCK B,5
#define INT D,2

//status light
#define LED2			B,0

#define RxIDLow	((RxID & 0x07) << 5)
#define RxIDHi	(uint8_t)(RxID >> 3)
