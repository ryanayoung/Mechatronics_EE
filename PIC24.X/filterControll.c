/* 
 * File:   filterControll.h
 * Author: Scott Briscoe
 *
 * Created on November 9, 2015, 8:19 PM
 */

#include "filterControll.h"

// initialize a pwm signal to control a band pass filter
void initFilterControll(void)
{
    // Setup PWM, and Direction bits to Output
    PWM_PIN_MODE = OUTPUT;

    // Setup re-mappable pins to Output Compare
    PWM_PIN_MAP_RP;

    // Set Output Compare mode to Disabled until init
    OC5CON1bits.OCM = 0b000;

    // Output Compare Register setup Duty Cycle
    OC5R = DEFAULT_PWM_PERIOD/2; // default PWM duty

    // Allow Output Compare to work when CPU is in Idle Mode
    OC5CON1bits.OCSIDL = 0;

    // Set the PWM Fault pin to reset (Only Used if OCM == 111)
    OC5CON1bits.OCFLT = 0;

    // Set the Clock Source for the Output Compare (Timer 2)
    OC5CON1bits.OCTSEL = 0b000;

    // set clock soarce from internal clock (FOSC/2)
    T2CONbits.TCS = 0b0;
    
    // timer clock prescaler
    T2CONbits.TCKPS = 0b00; // divide by 1

    // Set Output Compare Mode to Edge-Aligned PWM mode
    OC5CON1bits.OCM = 0x6;

    // Setup Period Value Register of Timer 2 with PWM frequency
    PR2 = DEFAULT_PWM_PERIOD; // default PWM period

    // Clear Timer 2 interrupt flag
    IFS0bits.T2IF = 0;

    // Start Timer 2 by setting the Timer On bit
    T2CONbits.TON = 0b1;
}

// for setting frequency between 25khz and 40khz for the filers
void setFilterControll(long systemClock, int pingerFreqKhz)
{
    
    unsigned int filterPeriod = 0;
    
    // will calculate and generate the period needed
    // abandoned this funtional code due to the intiger math truncation
    // and the tiny values generated for the timer
    
    //long timerClock             = systemClock/2; // 2 cycle instruction set and 1x divider
    //long filterFrequencyKhz     = (pingerFreqKhz * 328) / 10; // sets the 32.8:1 ratio   
    //filterPeriod   = timerClock / filterFrequencyKhz;
    
    //***********************//
    
    // created a lookup table because the timer values are so small
    // and the intiger math causes truncation and not rounding
    // are manually forcing frequencys to the closest acceptable values of the table

    // Setup Duty Cycle and Period
    // For frequencies between 40kHz and 39kHz
    if(pingerFreqKhz <= 40 && pingerFreqKhz >= 39)
    {
        filterPeriod = 12;
    }
    // For frequencies between 38kHz and 37kHz
    else if(pingerFreqKhz <= 38 && pingerFreqKhz >= 37)
    {
        filterPeriod = 13;
    }
    // For frequencies between 36kHz and 35kHz
    else if(pingerFreqKhz <= 36 && pingerFreqKhz >= 35)
    {
        filterPeriod = 14;
    }
    // For frequencies between 34kHz and 32kHz
    else if(pingerFreqKhz <= 34 && pingerFreqKhz >= 32)
    {
        filterPeriod = 14;
    }
    // For frequency of 31kHz
    else if(pingerFreqKhz == 31)
    {
        filterPeriod = 15;
    }
    // For frequencies between 30kHz and 29kHz
    else if(pingerFreqKhz <= 30 && pingerFreqKhz >= 29)
    {
        filterPeriod = 16;
    }
    // For frequency of 28kHz
    else if(pingerFreqKhz == 28)
    {
        filterPeriod = 17;
    }
    // For frequencies between 27kHz and 26kHz
    else if(pingerFreqKhz <= 27 && pingerFreqKhz >= 26)
    {
        filterPeriod = 18;
    }
    // For frequency of 25kHz
    else if(pingerFreqKhz == 25)
    {
        filterPeriod = 19;
    }
    else
    {
        filterPeriod = 19;
    }
    
    //***********************//
    
    // Output Compare Register setup Duty Cycle
    OC5R = filterPeriod / 2;
    // Setup Period Value Register of Timer 2 with PWM frequency
    PR2  = filterPeriod;

}


