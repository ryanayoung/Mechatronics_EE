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
#define LED1 D,3
#define LED2 D,4
#define LED3 D,5
#define LED4 D,6

//ADC inputs
#define V24_pin E,2
#define V6_pin E,3
#define V5_pin C,0
#define V20_pin C,1

//Filter 1
#define Rx0IDLow	((Rx0ID & 0x07) << 5)
#define Rx0IDHi		(uint8_t)(Rx0ID >> 3)

//Filter 1
#define Rx1IDLow	((Rx1ID & 0x07) << 5)
#define Rx1IDHi		(uint8_t)(Rx1ID >> 3)

//Filter 2
#define Rx2IDLow	((Rx2ID & 0x07) << 5)
#define Rx2IDHi		(uint8_t)(Rx2ID >> 3)
//Filter 3
#define Rx3IDLow	((Rx3ID & 0x07) << 5)
#define Rx3IDHi		(uint8_t)(Rx3ID >> 3)
//Filter 4
#define Rx4IDLow	((Rx4ID & 0x07) << 5)
#define Rx4IDHi		(uint8_t)(Rx4ID >> 3)

//Filter 5
#define Rx5IDLow	((Rx5ID & 0x07) << 5)
#define Rx5IDHi		(uint8_t)(Rx5ID >> 3)

#define RxMaskLow 0xE0
#define RXMaskHi  0x00

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
