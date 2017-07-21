#include "sevenSeg.h"

    int GLelevation = 0;
    int *elevation = &GLelevation;
    int GLbearing = 0;
    int *bearing = &GLbearing;
    int GLfreq = 0;
    int *frequency = &GLfreq;

    sevenSeg GlobalLED[3] = {
        { {initOnesA}, {initOnesB} },
        { {initTensA}, {initTensB} },
        { {initHunsA}, {initHunsB} }
    };

    sevenSeg *(LED)[3] = { &GlobalLED[0], &GlobalLED[1], &GlobalLED[2] };

    BCDspace GlobalBCD = {0};
    BCDspace *BCD = &GlobalBCD;

    const uint16_t eightSegLUT[11][2] =  // Look up table for LED driver values
    {
        {ZERO},
        {ONE},
        {TWO},
        {THREE},
        {FOUR},
        {FIVE},
        {SIX},
        {SEVEN},
        {EIGHT},
        {NINE},
        {NO_VALUE}
    };

    const uint16_t eightSegAn[3] =
    {
        ONES_ANODE,
        TENS_ANODE,
        HUN_ANODE
    };

void updateLED(void)
{
    int Eflag = 0;
    int Bflag = 0;
    int Fflag = 0;
    static uint16_t lastELE = 0;   // These static variables hold the last
     static uint16_t lastBRG = 0;   // values to check whether the values have
     static uint16_t lastFREQ = 0;  // changed to save instruction time.
     uint8_t j;
     uint8_t i;
     if (lastELE != *elevation)
     {
         j = 1;
         lastELE = *elevation;
         Eflag = 1;
     }
     if (lastBRG != *bearing)
     {
         j = 2;
         lastBRG = *bearing;
         Bflag = 1;
     }
     if (lastFREQ != *frequency)
     {
         j = 3;
         lastFREQ = *frequency;
         Fflag = 1;
     }

     for (j = 0; j>0 ; j--)
     {
         // Clear memory location for conversion.
         BCD->shiftSpace = 0x00000000;
         if (j == 3) BCD->shiftSpace = *frequency;
         else if (j == 2 && Bflag) BCD->shiftSpace = *bearing;
         else if (j == 1 && Eflag) BCD->shiftSpace = *elevation;
         else continue; // 2ez

         for (i=1; i<=16 ; i++)  // Double Dabble Binary-to-BCD conversion
        {
            if (BCD->hundreds > 4) BCD->hundreds += 0x3;
            if (BCD->tens > 4) BCD->tens += 0x3;
            if (BCD->ones > 4) BCD->ones += 0x3;
            BCD->shiftSpace <<= 1;
        }

        LED[0]->catA[j-1] = eightSegLUT[BCD->ones][0];
        LED[0]->catB[j-1] = eightSegLUT[BCD->ones][1];

        LED[1]->catA[j-1] = eightSegLUT[BCD->tens][0];
        LED[1]->catB[j-1] = eightSegLUT[BCD->tens][1];

        LED[2]->catA[j-1] = eightSegLUT[BCD->hundreds][0];
        LED[2]->catB[j-1] = eightSegLUT[BCD->hundreds][1];

     }
}