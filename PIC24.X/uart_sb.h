/* 
* File:   uart_sb.h
* Author: Scott Briscoe
*
* Created on November 17, 2015, 4:46 PM
*/

#ifndef UART_SB_H
#define	UART_SB_H

    #include <xc.h>

    // function prototypes
    void uartConfig(long systemClock, long baudRate);
    void uartWriteChar(char transmit);
    void uartWriteInt(int transmit);
    char uartReadChar();

#endif	/* UART_SB_H */

