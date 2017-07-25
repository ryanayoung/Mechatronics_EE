/* 
 * File:   hydraphoneComparators.h
 * Author: e217
 *
 * Created on November 19, 2015, 1:37 PM
 */

#ifndef HYDRAPHONECOMPARATORS_H
#define	HYDRAPHONECOMPARATORS_H

    #include <xc.h>

    #define INPUT  1
    #define OUTPUT 0
    #define ON  1
    #define OFF 0

    #define COMPARATOR1_PIN_MODE    TRISBbits.TRISB2
    //#define COMPARATOR2_PIN_MODE    TRISBbits.TRISB4 // not currently used
    #define COMPARATOR3_PIN_MODE    TRISBbits.TRISB14

    #define C1EVT 0X0100 // Bitmask for event flag in CMSTAT Module Status Reg
    //#define C2EVT 0X0200 // Bitmask for event flag in CMSTAT Module Status Reg
    #define C3EVT 0X0400 // Bitmask for event flag in CMSTAT Module Status Reg
    #define C1OUT 0X0001 // Bitmask for output flag in CMSTAT Module Status Reg
    //#define C2OUT 0X0002 // Bitmask for output flag in CMSTAT Module Status Reg
    #define C3OUT 0X0004 // Bitmask for output flag in CMSTAT Module Status Reg

    #define C1_EVENT    ((CMSTAT & C1EVT)!=0)
    //#define C2_EVENT    ((CMSTAT & C2EVT)!=0)
    #define C3_EVENT    ((CMSTAT & C3EVT)!=0)

    #define C1_C3_EVENT      (CMSTAT & 0b0000010100000000)
    #define C1_C3_OUTPUT     (CMSTAT & 0b0000000000000101)

    //**************************//
    // function prototypes
    void initHydraphoneComparators();
    
    

#endif	/* HYDRAPHONECOMPARATORS_H */

