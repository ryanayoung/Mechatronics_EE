/**
@author  Mechatronics RoboSub Team 2015
@brief   System level defines, variables, and hardware setup
*/
#ifndef __system__h
#define __system__h

// Includes
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

#include <stdbool.h>         /* For true/false definition */
#include "types.h"
#include "circularBuffer.h"

// Defines
#define SYS_FREQ        32000000L//8000000L
#define FCY             SYS_FREQ/2

// Variables
extern volatile uint32 msTicks;
extern CircularBuffer* ReceiveBuffer;
extern CircularBuffer* TransmitBuffer;

extern uint16 Heading1;  // Heading for Pinger 1 (10 bits)
extern uint16 AOI1;  // Angle of Incidence for Pinger 1 (10 bits)
extern uint8 Confidence1;  // Confidence in measurements for Pinger 1
extern uint16 Heading2;  // Heading for Pinger 2 (10 bits)
extern uint16 AOI2;  // Angle of Incidence for Pinger 2 (10 bits)
extern uint8 Confidence2;  // Confidence in measurements for Pinger 2
extern uint16 H1;  // Hydrophone 1 ADC value (10 bits)
extern uint16 H2;  // Hydrophone 2 ADC value (10 bits)
extern uint16 H3;  // Hydrophone 3 ADC value (10 bits)
extern uint16 H4;  // Hydrophone 4 ADC value (10 bits)
extern uint16 SpeedofSound;  // Speed of Sound in water
extern uint8 PingerFreq;  // Frequency of the Pinger (25kHz = 0x19, 33kHz = 0x21, 40kHz = 0x28)
extern uint8 FilterDuty;  // Duty Cycle of the PWM for tuning the filters
extern uint8 FilterPeriod;  // Period of the PWM for tuning the filters
extern uint16 Hydro1[20];  // Array to store Hydrophone 1 ADC values
extern uint16 Hydro2[20];  // Array to store Hydrophone 2 ADC values
extern uint16 Hydro3[20];  // Array to store Hydrophone 3 ADC values
extern uint16 Hydro4[20];  // Array to store Hydrophone 4 ADC values
extern uint16 UnFilt[20];  // Array to store unfiltered Hydrophone 3 ADC values
extern bool flag;

// Prototypes
extern void InitializeHardware(void);

#endif
