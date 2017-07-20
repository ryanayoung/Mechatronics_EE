/**
@author  Mechatronics RoboSub Team 2015
*/

// Includes
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "libpic30.h"      // Includes Delay Function
#include "crc.h"
#include "circularBuffer.h"
#include "packetCom.h"
#include "packets.h"
#include "austins_PIC24FJ32GB004_lib.h"
#include "sevenSeg.h"
#include "DOA.h"

// Defines
_CONFIG1(FWDTEN_OFF & JTAGEN_OFF)
_CONFIG2(IESO_ON  & OSCIOFNC_OFF & POSCMOD_HS & PLL96MHZ_ON & PLLDIV_DIV2 & FCKSM_CSDCMD & IOL1WAY_ON & FNOSC_FRCPLL & I2C1SEL_PRI)

#pragma config DSWDTEN = OFF
       

// Prototypes
//extern void SwitchSampling(void);

// Variables
volatile uint32 msTicks = 0;
CircularBuffer* ReceiveBuffer;
CircularBuffer* TransmitBuffer;

uint16 Heading1;  // Heading for Pinger 1 (10 bits)
uint16 AOI1;  // Angle of Incidence for Pinger 1 (10 bits)
uint8 Confidence1;  // Confidence in measurements for Pinger 1
uint16 Heading2;  // Heading for Pinger 2 (10 bits)
uint16 AOI2;  // Angle of Incidence for Pinger 2 (10 bits)
uint8 Confidence2;  // Confidence in measurements for Pinger 2
uint16 H1;  // Hydrophone 1 ADC value (10 bits)
uint16 H2;  // Hydrophone 2 ADC value (10 bits)
uint16 H3;  // Hydrophone 3 ADC value (10 bits)
uint16 H4;  // Hydrophone 4 ADC value (10 bits)
uint16 SpeedofSound;  // Speed of Sound in water
uint8 PingerFreq = 0x19;  // Frequency of the Pinger (25kHz = 0x19, 33kHz = 0x21, 40kHz = 0x28)
uint8 FilterDuty;  // Duty Cycle of the PWM for tuning the filters
uint8 FilterPeriod;  // Period of the PWM for tuning the filters

//extern uint16 Hydro1[20];  // Array to store Hydrophone 1 ADC values
//extern uint16 Hydro2[20];  // Array to store Hydrophone 2 ADC values
//extern uint16 Hydro3[20];  // Array to store Hydrophone 3 ADC values
//extern uint16 Hydro4[20];  // Array to store Hydrophone 4 ADC values
//extern uint16 UnFilt[20];  // Array to store unfiltered Hydrophone 3 ADC values
//extern uint16 sampleCount; // Number of samples taken
//bool bearingFlag = 0;  // 0 for heading, 1 for elevation


/* Main function containing the background tasks.*/

int16_t main(void)
{

    InitializeHardware();
    crcInit();

    PIN8_TRIS = OUTPUT;

    while(1)
    {
        PacketComState_Run();
        if (flag) phaseshift();
    }
}

// Switch sampling between heading and elevation for sample switching need to change the number of samples on the configurations
//void SwitchSampling(void)
//{
//    if (sampleCount >= 40000)
//    {
//        sampleCount = 0; // Reset the sample count
//
//        if (bearingFlag == 0)
//        {
//            bearingFlag = 1; // Switch to elevation (H2 and H3)
//            // latch one usb pin high
//            AD1CON1bits.ADON = 0; // Turn ADC off
//            AD1CON1bits.SAMP = 0; // Stop sampling
//            AD1CSSLbits.CSSL7 = 0; // Turn off Hydrophone 1 sampling
//            AD1CSSLbits.CSSL10 = 1; // Turn on Hydrophone 3 for elevation sampling
//            AD1CON1bits.ADON = 1; // Turn ADC on
//            AD1CON1bits.SAMP = 1; // Start sampling
//        }
//        else
//        {
//            bearingFlag = 0; // Switch to heading (H1 and H2)
//            // latch one usb pin low
//            AD1CON1bits.ADON = 0; // Turn ADC off
//            AD1CON1bits.SAMP = 0; // Stop sampling
//            AD1CSSLbits.CSSL10 = 0; // Turn off Hydrophone 3 sampling
//            AD1CSSLbits.CSSL7 = 1; // Turn on Hydrophone 1 for heading sampling
//            AD1CON1bits.ADON = 1; // Turn ADC on
//            AD1CON1bits.SAMP = 1; // Start sampling
//        }
//    }
//}


