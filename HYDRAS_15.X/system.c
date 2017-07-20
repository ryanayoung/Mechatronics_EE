// Includes
#include "system.h"
#include <uart.h>
#include "austins_PIC24FJ32GB004_lib.h"

// Defines
#define UART_BAUD 9600L

// Prototypes
static void TimeMsConfig(void);
static void UartConfig(void);
static void AdcConfig(void);
static void SevenSegInit(void);

void InitializeHardware(void)
{
    ReceiveBuffer = CircularBuffer_Create();
    TransmitBuffer = CircularBuffer_Create();

    TimeMsConfig();
    UartConfig();
    AdcConfig();
    SevenSegInit();

    PIN8_TRIS = OUTPUT;

    // Disable WatchDog Timer
    RCONbits.SWDTEN = 0;
    CLKDIVbits.CPDIV = 0;
    CLKDIVbits.RCDIV = 0;
#if 0

        /* Disable Watch Dog Timer */
        RCONbits.SWDTEN = 0;

        /* When clock switch occurs switch to Prim Osc (HS, XT, EC)with PLL */
        __builtin_write_OSCCONH(0x03);  /* Set OSCCONH for clock switch */
        __builtin_write_OSCCONL(0x01);  /* Start clock switching */
        while(OSCCONbits.COSC != 0b011);

        /* Wait for Clock switch to occur */
        /* Wait for PLL to lock, if PLL is used */
        /* while(OSCCONbits.LOCK != 1); */

#endif
}

void TimeMsConfig(void)
{
    // Setup Timer 1 interrupt
    PR3 = 0x3E80;//0x1F40;//0xFA0;	 // Set to 4000 (0xFA0), since 4 MHz / 4k = 1kHz
    IPC2bits.T3IP = 5;	 // Set interrupt priority
    T3CONbits.TON = 1;	 // Turn on the Timer
    IFS0bits.T3IF = 0;	 // Reset interrupt flag
    IEC0bits.T3IE = 1;	 // Turn on the Timer3 interrupt

//    // Setup Timer 3 interrupt
//    T1CON = 0x0000;         // Clear bits in Timer configuration register and stop clock.
//    TMR1 = 0xFFFF;          // Clear the Timer 1 register.
//    PR1 = 0x0001;           // Set Timer 1 period register to utilize all 16 bits.
//    IFS0bits.T1IF = 0;      // Clear the Timer 1 Interrupt Flag (prior to enabling the interrupt).
//    IPC0bits.T1IP = 1;      // Set Timer 1 to lowest priority
//    IEC0bits.T1IE = 1;      // Enable the Timer 1 interrupt.
//    T1CONbits.TON = 1;      // Enable the Timer 1 module.
}

void UartConfig(void)
{
    // Pin 4 Tx, Pin 5 Rx
    RPOR12bits.RP24R = 5;
    TRISCbits.TRISC8 = 0;
    RPINR19bits.U2RXR = 25;
    TRISCbits.TRISC9 = 1;

    // Set Baud Rate to 9600
    U2BRG = FCY / 16 / UART_BAUD - 1;//0x19;

    // Allow operation in idle mode
    U2MODEbits.USIDL = 0;
    U2MODEbits.WAKE = 0;

    // Enable Uart 2
    U2MODEbits.UARTEN = 1;

    // Enable Interrupt when there is 1 space in the Tx Buffer
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.UTXISEL0 = 0;
    // Enable Interrupt when Rx Buffer is not Empty
    U2STAbits.URXISEL = 0;

    // Enable Transmitter
    U2STAbits.UTXEN = 1;

    // Reset RX interrupt flag
    IFS1bits.U2RXIF = 0;
    // Reset TX interrupt flag
    IFS1bits.U2TXIF = 0;

    // Enable Tx Interrupt
    IEC1bits.U2TXIE = 1;
    // Enable Rx Interrupt
    IEC1bits.U2RXIE = 1;
}

void AdcConfig(void)
{
    // Need ADC on Pin 24 (RB3) - Hydrophone 4 Filtered AN5
    //             Pin 26 (RC1) - Hydrophone 1 Filtered AN7 (heading sampling)
    //             Pin 14 (RB14) - Hydrophone 3 Filtered AN10 (elevation sampling)
    //             Pin 11 (RB13) - Hydrophone 3 UnFiltered AN11
    //             Pin 36 (RC3) - Hydrophone 2 Filtered AN12 (reference)

    // Set Analog Channels to Correct Pins, Enable Analog - Disable I/O
    AD1PCFGbits.PCFG7 = 0; // Set AN7 as analog
    AD1PCFGbits.PCFG10 = 0; // Set AN10 as analog
    AD1PCFGbits.PCFG12 = 0; // Set AN12 as analog

    // Set Analog Pins to Input Channel
    AD1CSSLbits.CSSL7 = 1; // AN7 as input channel for sequential reading
    AD1CSSLbits.CSSL10 = 1; // AN10 as input channel for sequential reading
    AD1CSSLbits.CSSL12 = 1; // AN12 as input channel for sequential reading

    // Set TRIS To Input
    TRISBbits.TRISB3 = 1; // Set RB3 as input
    TRISBbits.TRISB14 = 1; // Set RB14 as input
    TRISCbits.TRISC3 = 1; // Set RC3 as input

    AD1CON1bits.ADSIDL = 0; // Continue in Idle Mode
    AD1CON1bits.FORM = 0b00; // Integer Data Output [0000 00dd dddd dddd]
    AD1CON1bits.SSRC = 0b111; // Internal Counter for Trigger Source, Auto Convert
    AD1CON1bits.ASAM = 1; // Sample Auto-Start, auto begins sampling again

    AD1CON2bits.VCFG = 0b000; // Voltage Reference AVdd, AVss
    AD1CON2bits.CSCNA = 1; // Channels specified by AD1CSSL register, Scan for inputs
    AD1CON2bits.SMPI = 0b0010; // Interrupt after completion of 3 sample(s)
    AD1CON2bits.BUFM = 0; // Buffer configured as 16-word Single Buffer
    AD1CON2bits.ALTS = 0; // Only use MUX A for sampling

    AD1CON3bits.ADRC = 0; // Conversion Clock from system clock
    AD1CON3bits.SAMC = 0b00001; // Auto-sample time bits 1 Tad - Set Sample Period
    AD1CON3bits.ADCS = 0b00000001; // Conversion Clock Period 1 * Tcy

    AD1CHSbits.CH0SA = 0b00000; // Positive input is AN0
    AD1CHSbits.CH0NA = 1; // Negative input is AN1 GND Reference

    IPC3bits.AD1IP = 0b111; // Set ADC interrupt to have priority over UART

    IFS0bits.AD1IF = 0; // Clear interrupt flag
    IEC0bits.AD1IE = 1; // Enable interrupt flag

    AD1CON1bits.ADON = 1; // Turn ADC on
    AD1CON1bits.SAMP = 1; // Start sampling
}

void SevenSegInit(void)
{

    TRISAbits.TRISA2 = 0;   // Pin 30 (RA2) set as output (CATHODE A)
    TRISAbits.TRISA3 = 0;   // Pin 31 (RA3) set as output (CATHODE B)
    TRISAbits.TRISA4 = 0;   // Pin 34 (RA4) set as output (CATHODE C)
    TRISAbits.TRISA7 = 0;   // Pin 13 (RA7) set as output (CATHODE D)
    TRISAbits.TRISA8 = 0;   // Pin 32 (RA8) set as output (CATHODE E)
    TRISAbits.TRISA10 = 0;  // Pin 12 (RA10) set as output (CATHODE F)
    TRISBbits.TRISB4 = 0;   // Pin 33 (RB4) set as output (CATHODE G)
    TRISBbits.TRISB5 = 0;   // Pin 41 (RB5) set as output (CATHODE DP)
    TRISBbits.TRISB7 = 0;   // Pin 43 (RB7) set as output (ANODE 1- 10^2)
    TRISBbits.TRISB8 = 0;   // Pin 44 (RB8) set as output (ANODE 2- 10^1)
    TRISBbits.TRISB9 = 0;   // Pin 1 (RB9) set as output (ANODE 3- 10^0)
}