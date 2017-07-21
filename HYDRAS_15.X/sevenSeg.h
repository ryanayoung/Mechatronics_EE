/*
 * File:   sevenSeg.h
 * Author: joe
 *
 * Created on June 8, 2015, 4:18 PM
 */

#ifndef SEVENSEG_H
#define	SEVENSEG_H
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif


#define initOnesA  0x0584, 0x049C, 0x0584
#define initOnesB  0x0030, 0x0030, 0x0030
#define initTensA  0x0580, 0x0100, 0x0100
#define initTensB  0x0000, 0x0010, 0x0010
#define initHunsA  0x0584, 0x0590, 0x0504
#define initHunsB  0x0010, 0x0010, 0x0010

#define alphaB 0x0590, 0x0010
#define alphaE 0x0584, 0x0010
#define alphaF 0x0504, 0x0010
#define alphaR 0x0100, 0x0010
#define alphaG 0x049C, 0x0010
#define alphaL 0x0580, 0x0000

#define NO_VALUE 0x0000, 0x0010
#define ZERO    0x059C, 0x0000
#define ONE     0x0018, 0x0000
#define TWO     0x018C, 0x0010
#define THREE   0x009C, 0x0010
#define FOUR    0x0418, 0x0010
#define FIVE    0x0494, 0x0010
#define SIX     0x0594, 0x0010
#define SEVEN   0x001C, 0x0000
#define EIGHT   0x059C, 0x0010
#define NINE    0x049C, 0x0010
#define DECIMAL_PT  0x0020
#define ONES_ANODE 0x0180
#define TENS_ANODE 0x0280
#define HUN_ANODE 0x0300


    extern void updateLED(void);

    typedef struct  // data structure for seven segment display
    {
        uint16_t catA[3];   // [0] Elevation, [1] Bearing, [2] Freq decomposed
        uint16_t catB[3];   //  into values that drive the 7 segment display.
    } sevenSeg;

    typedef union
    {
        uint32_t shiftSpace;
        struct
        {
            uint16_t original : 16,
                    ones : 4,
                    tens : 4,
                    hundreds : 4,
                    lead : 4;
        };
    }BCDspace;

    /******** GLOBAL VARIABLES/POINTERS ********/
    // LED variables
    extern sevenSeg GlobalLED[3];
    extern sevenSeg *(LED)[3];

    // BCD conversion variables
    extern BCDspace GlobalBCD;
    extern BCDspace *BCD;

    // DEBUGGING variables
    extern int GLelevation;
    extern int *elevation;
    extern int GLbearing;
    extern int *bearing;
    extern int GLfreq;
    extern int *frequency;

   extern const uint16_t eightSegLUT[11][2];  // Look up table for LED driver values
   extern const uint16_t eightSegAn[3];     // LUT for anode pins
    


#ifdef	__cplusplus
}
#endif

#endif	/* SEVENSEG_H */

