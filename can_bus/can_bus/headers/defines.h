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
#define LED2			B,0

#define RxIDLow	((RxID & 0x07) << 5)
#define RxIDHi	(uint8_t)(RxID >> 3)

//Uart Receive statemachine states
#define s_RxIDH 0x10
#define s_RxIDL 0x20
#define s_Rxdata1 0x01
#define s_Rxdata2 0x02
#define s_Rxdata3 0x03
#define s_Rxdata4 0x04
#define s_Rxdata5 0x05
#define s_Rxdata6 0x06
#define s_Rxdata7 0x07
#define s_Rxdata8 0x08
