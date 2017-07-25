

#include "hydraphoneComparators.h"

//**************************************************//

void initHydraphoneComparators()
{
    COMPARATOR1_PIN_MODE = INPUT;  // Disable Digital Output on port pin
    //COMPARATOR2_PIN_MODE = INPUT;  // Disable Digital Output on port pin
    COMPARATOR3_PIN_MODE = INPUT;  // Disable Digital Output on port pin
    

    IEC1bits.CMIE = 0; // IE Off so no interrupt occurs from setup
    
    // setup comparator 1
    CM1CONbits.COE   = 0; // Disable output pin
    CM1CONbits.CPOL  = 0; // Standard sense. +In High ==> Out High
    //CM1CONbits.EVPOL = 1; // Event detected on output edge falling
    CM1CONbits.EVPOL = 2; // Event detected on output edge rising
    //CM1CONbits.EVPOL = 3; // Event detected on rising and falling
    CM1CONbits.CREF  = 1; // +IN is internal CVRef
    CM1CONbits.CCH   = 0; // -IN is C1INB Pin (RB2 pin)
    CM1CONbits.CON   = 1; // Turn Comparator ON
    CM1CONbits.CEVT  = 0; // Clear Event Flag 
    
    // setup comparator 3
    CM3CONbits.COE   = 0; // Disable output pin
    CM3CONbits.CPOL  = 0; // Standard sense. +In High ==> Out High
    //CM3CONbits.EVPOL = 1; // Event detected on output edge falling
    CM3CONbits.EVPOL = 2; // Event detected on output edge rising
    //CM3CONbits.EVPOL = 3; // Event detected on rising and falling
    CM3CONbits.CREF  = 1; // +IN is internal CVRef
    CM3CONbits.CCH   = 0; // -IN is C1INB Pin (RB14 pin)
    CM3CONbits.CON   = 1; // Turn Comparator ON
    CM3CONbits.CEVT  = 0; // Clear Event Flag
    
    CVRCON = 0x8C; // CVRef = (1/2) * (AVdd - AVss)
    
    IFS1bits.CMIF = 0; // Clear IF after set-up
    
    //******************//
    
    // initialize the events and variables
    CM1CONbits.CEVT  = 0; // Clear Event Flag 
    CM3CONbits.CEVT  = 0; // Clear Event Flag
    
    
}