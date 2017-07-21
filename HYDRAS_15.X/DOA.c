#include "DOA.h"
#include "system.h"
#include "direction.h"
#include "types.h"

int markA = 0;
int markB = 0;
int32_t deadtimeA = 0;
int32_t deadtimeB = 0;
int32_t BearingThreshA = 0;
int32_t BearingThreshB = 0;
int32_t BearingholderThreshold = 150000;
int32_t pingtrackerBearing = 0;
int32_t PingtrackerThreshold = 150000;
int BearingphaseshiftA = 0;
int BearingphaseshiftB = 0;
int BearingElementNumberA = 5;
int BearingElementNumberB = 5;

void phaseshift(void)
{
    //if (hydrophoneA > 0.21)
    if (H1 > 21)
    {
        if (markA == 1)
        {
            if ((pingtrackerBearing < PingtrackerThreshold) || (pingtrackerBearing > PingtrackerThreshold + 150000))
            {
                pingtrackerBearing = 1;

                if (BearingThreshA > 0 )
                {
                    BearingphaseshiftA = BearingThreshA - 1;
                    if(BearingphaseshiftA < 10)
                    {
                    Heading1 = DOA(BearingphaseshiftA, 1);
                    }
                    BearingThreshA = 0;
                }
                else
                {
                    BearingThreshA = BearingThreshA + 1;
                    BearingElementNumberA = 0;
                }
            }
            else
            {

                if (BearingThreshB > 0)
                {
                    BearingphaseshiftB = BearingThreshB - 1  ;
                    if(BearingphaseshiftB < 10)
                    {
                    Heading2 = DOA(BearingphaseshiftB, 1);
                    }
                    BearingThreshB = 0;
                }
                else
                {
                    BearingThreshB = BearingThreshB + 1;
                    BearingElementNumberB = 0;
                }
            }
        }

        markA = 0;
        deadtimeA = 0;
    }
    else
    {
        deadtimeA = deadtimeA + 1;

        if (deadtimeA > 10000)
        {
            markA = 1;
        }
        else
        {
            markA = 0;
        }
    }

    //if (hydrophoneB > 0.2)
    if (H2 > 21)
    {
        if (markB == 1)
        {
            if ((pingtrackerBearing < PingtrackerThreshold) || (pingtrackerBearing > PingtrackerThreshold + 150000))
            {
                pingtrackerBearing = 1;

                if (BearingThreshA > 0)
                {
                    BearingphaseshiftA = BearingThreshA - 1;
                    if(BearingphaseshiftA < 10)
                    {
                    Heading1 = DOA(BearingphaseshiftA, 0);
                    }
                    BearingThreshA = 0;
                }
                else
                {
                    BearingThreshA = BearingThreshA + 1;
                    BearingElementNumberA = 1;
                }
            }
            else
            {
                if (BearingThreshB > 0)
                {
                    BearingphaseshiftB = BearingThreshB - 1;
                    if(BearingphaseshiftB < 10)
                    {
                    Heading2 = DOA(BearingphaseshiftB, 0);
                    }
                    BearingThreshB = 0;
                }
                else
                {
                    BearingThreshB = BearingThreshB + 1;
                    BearingElementNumberB = 1;
                }
            }

            markB = 0;
        }

        deadtimeB = 0;
    }
    else
    {
        deadtimeB = deadtimeB + 1;

        if (deadtimeB > 10000)
        {
            markB = 1;
        }
        else
        {
            markB = 0;
        }
    }

        if (BearingThreshA > 0)
        {
            if (BearingThreshA == BearingholderThreshold)
            {
                BearingThreshA = 0;
            }
            else
            {
                BearingThreshA = BearingThreshA + 1;
            }
        }

        if (BearingThreshB > 0)
        {
            if (BearingThreshB == BearingholderThreshold)
            {
                BearingThreshB = 0;
            }
            else
            {
                BearingThreshB = BearingThreshB + 1;
            }
        }

        if (pingtrackerBearing > 0)
        {
            pingtrackerBearing = pingtrackerBearing + 1;
        }

    flag = 0;
}

double DOA(int phaseshift, int Element)
{

    int angle;

    if (Element == 1)
    {
        angle = angleLUT[phaseshift];
    }
    else
    {
        angle = 360 - angleLUT[phaseshift];
    }

    return angle;
}
