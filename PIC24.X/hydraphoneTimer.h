/* 
 * File:   hydraphoneTimer.h
 * Author: Scott Briscoe
 *
 * Created on November 17, 2015, 4:35 PM
 */

#ifndef HYDRAPHONETIMER_H
#define	HYDRAPHONETIMER_H

    #include <xc.h>

    

    // function prototypes
    long initHydraphoneTimer(long systemClock, long periodMilliseconds);
    void hydraphoneTimer_clear();
    long hydraphoneTimer_read();


#endif	/* HYDRAPHONETIMER_H */

