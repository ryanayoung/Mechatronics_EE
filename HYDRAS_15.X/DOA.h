/*
 * File:   DOA.h
 * Author: waiti_000
 *
 * Created on July 14, 2015, 3:57 PM
 */

#ifndef DOA_H
#define	DOA_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"
    
extern int markA;
extern int markB;
extern int32_t deadtimeA;
extern int32_t deadtimeB;
extern int32_t BearingThreshA;
extern int32_t BearingThreshB;
extern int32_t BearingholderThreshold;
extern int32_t pingtrackerBearing;
extern int32_t PingtrackerThreshold;
extern int BearingphaseshiftA;
extern int BearingphaseshiftB;
extern int BearingElementNumberA;
extern int BearingElementNumberB;

extern void phaseshift(void);
extern double DOA(int phaseshift, int Element);

#ifdef	__cplusplus
}
#endif

#endif	/* DOA_H */

