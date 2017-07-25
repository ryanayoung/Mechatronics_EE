

#include "hydraphoneTimer.h"

//**************************************************//

// initialize a timer to keep track of expected hydraphone arrival
// return the period calculated
long initHydraphoneTimer(long systemClock, long periodMilliseconds)
{
    long timerClock = systemClock / (2 * 256);
    long period     = (timerClock * periodMilliseconds) / 1000 ;
    
    // set clock source from internal clock (FOSC/2)
    T4CONbits.TCS = 0b0;
    
    // setup as a 32bit timer
    // this timer uses timer4 and timer5 for one 32bit timer
    T4CONbits.T32 = 0b1;
    
    // timer clock prescaler
    T4CONbits.TCKPS = 0b11; // divide by 256

    // Setup Period Value Register of Timer 4 and Timer5
    PR4 =  period & 0x0000FFFF; //set the lower timer period
    PR5 = (period & 0xFFFF0000) >> 16; //set the higher timer period

    // Clear Timer 1 interrupt flag
    //IFS0bits.T1IF = 0;

    // Start Timer 4 by setting the Timer On bit
    T4CONbits.TON = 0b1;
    
    return period;
}

//**************************************************//

void hydraphoneTimer_clear()
{
    //reset the timer to zero
    TMR4 = 0;
    TMR5 = 0; 
}

//**************************************************//

long hydraphoneTimer_read()
{
    //long low  = TMR4;
    long high = TMR5;
    //long combined = ((high << 16) | low);
    
    //return combined;
    return  ((high << 16) | TMR4);
}