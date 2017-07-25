/* 
 * File:   filterControll.h
 * Author: Scott Briscoe
 *
 * Created on November 9, 2015, 8:19 PM
 */

#ifndef FILTERCONTROLL_H
#define	FILTERCONTROLL_H


    // includes
    #include <xc.h> // for processor specific defines

    //**************************************************//
    // type defines and settings

    #define INPUT  1
    #define OUTPUT 0
    #define ON  1
    #define OFF 0

    #define PWM_PIN_MODE        TRISBbits.TRISB7
    #define PWM_PIN_MAP_RP      RPOR3bits.RP7R = 22; // RB7 is RP7

    #define DEFAULT_PWM_PERIOD      19 //for 20khz

    #define FEQUENCY_FILTER_LOW     25 //khz signal
    #define FEQUENCY_FILTER_HIGH    40 //khz signal


    //**************************************************//

    // function prototypes
    void initFilterControll(void);
    void setFilterControll(long systemClock, int pingerFreqKhz);


#endif

