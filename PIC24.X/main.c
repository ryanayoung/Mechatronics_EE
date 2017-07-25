/* 9768
* File:   main.c
* Author: Scott Briscoe
*
* Created on September 28, 2015, 9:21 PM
*/
//**************************************************//
// include system libraries

#include <xc.h>

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>        // Includes uint16_t definition
#include <stdbool.h>       // Includes true/false definition

// Declares For Delay Function
#define SYS_FREQ        32000000L // verified by using the clock out pin
#define FCY             SYS_FREQ/2
#include <libpic30.h>   

//**************************************************//
// include our custom libraries

//#include "hydraphoneTimer.h"
#include "hydraphoneComparators.h"
#include "filterControll.h"
#include "uart_sb.h"

//**************************************************//
// configuration settings

// setup oscillator as primary oscillator
// see table 6-1 of oscillator datasheet
#pragma config POSCMOD  = 01     // XT external oscillator
#pragma config FNOSC    = 0b011  // PLL on

// oscillator datasheet 6.6.2
// divide by 2 to turn 8mhz to the 4mhz input requirement for pll
#pragma config PLLDIV   = 0b001  // divide by 2

#pragma config FWDTEN   = OFF   // Watchdog Timer off
#pragma config JTAGEN   = OFF   // JTAG port is disabled

//**************************************************//
// type defines for this program

#define INPUT  1
#define OUTPUT 0
#define ON  1
#define OFF 0

// for senior design etched board
#define LED1_PIN_MODE    TRISBbits.TRISB9
#define LED1_PIN_STATE   LATBbits.LATB9

#define LED2_PIN_MODE    TRISCbits.TRISC6
#define LED2_PIN_STATE   LATCbits.LATC6

// for clock output
#define CLOCKOUT_MODE   TRISBbits.TRISB13


#define UART_BAUD                   115200L // baud rate
#define DEFAULT_FILTER_FREQUENCY    40 //khz

#define HYDRAPHONE_MIN_DEADTIME         300 // milliseconds of no comparator readings before listening to hydraphones
#define HYDRAPHONE_MAX_TIME_DIFFERENCE  200 // microseconds. Worst +- time difference we can expect from actuall time measurements
                                            // this is assuming the hydraphones are 12inches apart

#define SAMPLE_WINDOW_SIZE              8000 //samples at 1us per sample

#define ERROR_BETWEEN_SAMPLING          0x01 // error when a event happens in between sampling windows
#define ERROR_MISS_DETECT               0x02 // only one hydraphone was detected in the window
#define ERROR_BAD_TIME                  0x03 // value measured does not seem like a real time measurement, larger than what we expected
#define ERROR_NONE                      0x00 // no errors, have a good reading

#define END_MESSAGE                     0x0D //corresponds to the carrage retun as well


//**************************************************//
// function prototypes

int main(int argc, char** argv);

//**************************************************//
// measure the time difference of two hydrophone signal arriving
// send the calculated time back to the main pc via UART
int main(int argc, char** argv)
{
    //********//
	// define all local variables for this program
	
	int count; 							// for loop counting
    char samples[SAMPLE_WINDOW_SIZE]; 	// stores hydrophone comparator samples
    int deadtimeCount = 0;              // keeps track of current deadtime
	char error; 						// stores error code
    
	int lastReading; 	// for remembering past comparator events
    int currentReading; // for storing current comparator events
    
    int hydraphone1Time = -1; // stores arrival time of a hydrophone
    int hydraphone2Time = -1; // stores arrival time of a hydrophone
    
    int arrivalTimeDifference; //stores the difference in arrival times
    
	
    //********//
    // Setup Oscillator as Primary External With PLL
    // see register 6-1 of oscillator datasheet
    OSCCONbits.COSC = 0b011;
   
    // set cpu peripheral clock
    // see register 6-2 of oscillator datasheet
    CLKDIVbits.DOZEN  = 0b0;   // 1:1 ratio
    CLKDIVbits.DOZE   = 0b000; // 1:1 ratio
    CLKDIVbits.RCDIV  = 0b000; // divide by 1
    
    CLKDIVbits.CPDIV  = 0b00;  // 32MHz select
    CLKDIVbits.PLLEN  = 0;     // disable the 96mhz pll
    
    // REFOCON - route clock to an external io pin
	// use this pin to verify your clock is setup properly
    REFOCONbits.ROEN  = 1; 		// enable the clock output pin PB13
    REFOCONbits.ROSEL = 0; 		// use the clock source for the rest of the device
    REFOCONbits.RODIV = 0b0000;
    CLOCKOUT_MODE = OUTPUT; 	// setup pin as a output
    
    //********//
    // initialize hardware components
    
    LED1_PIN_MODE = OUTPUT;
    LED2_PIN_MODE = OUTPUT;
    LED1_PIN_STATE  = OFF;
    LED2_PIN_STATE  = OFF;
   
    initHydraphoneComparators();
    
    initFilterControll();
    setFilterControll(SYS_FREQ, DEFAULT_FILTER_FREQUENCY);
    
    uartConfig(SYS_FREQ, UART_BAUD);
    
    //********//
    
    // blink leds on startup
    for(count=0; count<2; ++count)
    {
        LED1_PIN_STATE  = ON;
        LED2_PIN_STATE  = ON;
        __delay_ms(500);
        LED1_PIN_STATE  = OFF;
        LED2_PIN_STATE  = OFF;
        __delay_ms(500) 
    }
    
    // main algorithm here
    //********//
    while(1)
    {
       
        // clear variables for next loop around
        hydraphone1Time = -1;
        hydraphone2Time = -1;
        arrivalTimeDifference = 0;
        error = ERROR_NONE;

        
        //********//
        // must have at least the deadtime of no comparator input before continuing

        deadtimeCount = 0; // clear the deadtime counter

        // clear the comparator event flags, very important
        CM1CONbits.CEVT  = 0; // Clear Event Flag 
        CM3CONbits.CEVT  = 0; // Clear Event Flag

        // wait here until a minimum deadtime has passed with no comparator events
        while(deadtimeCount < HYDRAPHONE_MIN_DEADTIME)
        {
            __delay_ms(1); // sets the sample rate
            ++deadtimeCount;

            // if a comparator event detected restart the count
            if (C1_C3_EVENT != 0)
            {
                deadtimeCount = 0; // clear the deadtime counter

                // clear the comparator event flags, very important
                CM1CONbits.CEVT  = 0; // Clear Event Flag 
                CM3CONbits.CEVT  = 0; // Clear Event Flag
            }

        }
            

       //********//
        
       // led on during saving
       LED2_PIN_STATE  = ON; 

	   // clear the comparator event flags, very important
       CM1CONbits.CEVT  = 0; // Clear Event Flag 
       CM3CONbits.CEVT  = 0; // Clear Event Flag
	   
	   // fire off bursts of samples until we know we captured data somewhere in the samples
       // will do samples as long as no events detected
	   while(C1_C3_EVENT == 0)
	   {
		   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		   // fire off all the samples as fast as we can
		   // this section of code needs to be VERY well written
		   // check disassembly code to make sure this loops in less than 16 assembly instructions
		   // this gives us our 1us window (1mhz sample rate)
		   
		   // code verified to be exactly 16 assembly instructions using C16 compiler
		   // do not change unless you really know what your doing
		   // unless you changed the clock speed your not going to do any better than it is now
		   for(count=0; count<SAMPLE_WINDOW_SIZE; ++count)
		   {
			   samples[count] = (C1_C3_EVENT >> 8);
			   //Nop(); // so we can set a breakpoint here during debugging
		   }
		   
		   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		   
       }
       
       
        //********//
       // led off during sending and waiting 
       LED2_PIN_STATE  = OFF;
       
       
       // check the first sample for a error
       // if the samples do not start out as all zeroes, we have a problem
	   // have had a comparator event in between sampling windows
       if(samples[0] != 0)
       {
           error = ERROR_BETWEEN_SAMPLING; // sets the error number
       }
	   // we have events, need to go parse the data for information
       else
       {
           lastReading = samples[0]; //initialize for looping
           
           // go back through the array and save the time difference between changes
           for(count=1; count<SAMPLE_WINDOW_SIZE; ++count)
           {
               currentReading = samples[count];
               
               // current sample is different than the last
               if (currentReading != lastReading)
               {
                   //check if its the first hydrophone and haven't detected it before
                   if( (currentReading & 0b0001) != 0 && hydraphone1Time == -1)
                   {
                       // save the time the hydrophone tripped
                       hydraphone1Time = count;
                   }
                   
                   //check if its the second hydrophone and haven't detected it before
                   if( (currentReading & 0b0100) != 0 && hydraphone2Time == -1)
                   {
                       // save the time the hydrophone tripped
                       hydraphone2Time = count;
                   }
                   
                   // remember this state for the next loop around
                   lastReading = currentReading;
                   
               }
               // last sample same as current sample
               else
               {
                   // do nothing, will just continue on
               }

           }
		   
		    // see if there was a error
		    // only one hydrophone was detected when parsing the data
		    if( (hydraphone1Time == -1 || hydraphone2Time == -1))
		    {
				error = ERROR_MISS_DETECT;
		    }
       
       }
       
      
	   // if no errors calculate the time difference
       if(error == ERROR_NONE)
       {
            // calculate the arrival time difference
            arrivalTimeDifference = (hydraphone2Time - hydraphone1Time);
            
            // make sure the number calculated is within the range of realistic values the system should measure
            // flag a error if it is out of range
            if( abs(arrivalTimeDifference) > HYDRAPHONE_MAX_TIME_DIFFERENCE )
            {
                error = ERROR_BAD_TIME;
            }
       }
       
       // send a UART message with the current calculated time and error status
       //uartWriteChar('E');
       uartWriteChar( error );
       uartWriteInt( arrivalTimeDifference );
       uartWriteChar(END_MESSAGE);
       
       //Nop(); // so we can set a breakpoint here during debugging
               
    }//end infinite loop

}//end main

//**************************************************//

