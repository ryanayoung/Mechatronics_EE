

#include "uart_sb.h"


//**************************************************//

void uartConfig(long systemClock, long baudRate)
{
    // Pin 4 Tx, Pin 5 Rx
    RPOR9bits.RP19R = 5;      //remap out to U2TX
    RPINR19bits.U2RXR = 20;   //remap in  to RP20
   
    TRISCbits.TRISC3 = 0;   // set tx out
    TRISCbits.TRISC4 = 1;   // set rx in

    U2MODEbits.BRGH = 1; //fast speed
    
    // Set Baud Rate to UART_BAUD
    U2BRG = ((systemClock/2) / (4 * baudRate)) - 1; //0x19; if 9600 baud

    // Allow operation in idle mode
    U2MODEbits.USIDL = 0;
    U2MODEbits.WAKE = 0;

    // Enable Uart 2
    U2MODEbits.UARTEN = 1;

    // Enable Transmitter
    U2STAbits.UTXEN = 1;
    
}

//**************************************************//

void uartWriteChar(char transmit)
{
    // while the buffer has data in it
    while(U2STAbits.TRMT == 0)
    {
        // do nothing
    }
    
    // send a byte
    U2TXREG = transmit;
    
}

void uartWriteInt(int transmit)
{
    // while the buffer has data in it
    while(U2STAbits.TRMT == 0)
    {
        // do nothing
    }
   
    
    // send the upper byte
    U2TXREG = (transmit & 0b1111111100000000) >> 8;
    
    // while the buffer has data in it
    while(U2STAbits.TRMT == 0)
    {
        // do nothing
    }
    
    // send the lower byte
    U2TXREG = transmit & 0b0000000011111111;
    
}

//**************************************************//

char uartReadChar()
{
    // while the buffer has no data
    while(U2STAbits.URXDA == 0)
    {
        // do nothing
    }
    
    return U2RXREG;
    
}