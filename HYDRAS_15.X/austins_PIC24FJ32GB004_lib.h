/* 
 * File:   austins_PIC24FJ32GB004_lib.h
 * Author: Austin
 *
 * Created on January 31, 2015, 12:19 AM
 */

#ifndef AUSTINS_PIC24FJ32GB004_LIB_H
#define AUSTINS_PIC24FJ32GB004_LIB_H

/*
REGISTER NUMBERS ON PIC24J32GB004 (USED FOR TRIS REGISTER)
+------+---------+------+---------+
| PINS | REG NUM | PINS | REG NUM |
+------+---------+------+---------+
|   1  |    RB9  |  23  |    RB2  |
|   2  |    RC6  |  24  |    RB3  |
|   3  |    RC7  |  25  |    RC0  |
|   4  |    RC8  |  26  |    RC1  |
|   5  |    RC9  |  27  |    RC2  |
|   6  |  *NONE* |  28  |  *NONE* |
|   7  |  *NONE* |  29  |  *NONE* |
|   8  |   RB10  |  30  |    RA2  |
|   9  |   RB11  |  31  |    RA3  |
|  10  |  *NONE* |  32  |    RA8  |
|  11  |   RB13  |  33  |    RB4  |
|  12  |  *NONE* |  34  |    RA4  |
|  13  |  *NONE* |  35  |    RA9  |
|  14  |   RB14  |  36  |    RC3  |
|  15  |   RB15  |  37  |    RC4  |
|  16  |  *NONE* |  38  |    RC5  |
|  17  |  *NONE* |  39  |  *NONE* |
|  18  |  *NONE* |  40  |  *NONE* |
|  19  |    RA0  |  41  |    RB5  |
|  20  |    RA1  |  42  |  *NONE* |
|  21  |    RB0  |  43  |    RB7  |
|  22  |    RB1  |  44  |    RB8  |
+------+---------+------+---------+
*/

//INPUT/OUTPUT
#define INPUT  1
#define OUTPUT 0

//ON/OFF
#define ON  1
#define OFF 0

//PIN NUMBER TO TRI STATE REGISTER (TRIS) (EX: PIN26_TRIS = OUTPUT)
#define PIN1_TRIS  TRISBbits.TRISB9
#define PIN2_TRIS  TRISCbits.TRISC6
#define PIN3_TRIS  TRISCbits.TRISC7
#define PIN4_TRIS  TRISCbits.TRISC8
#define PIN5_TRIS  TRISCbits.TRISC9
#define PIN8_TRIS  TRISBbits.TRISB10
#define PIN9_TRIS  TRISBbits.TRISB11
#define PIN11_TRIS TRISBbits.TRISB13
#define PIN12_TRIS TRISAbits.TRISA10
#define PIN13_TRIS TRISAbits.TRISA7
#define PIN14_TRIS TRISBbits.TRISB14
#define PIN15_TRIS TRISBbits.TRISB15
#define PIN19_TRIS TRISAbits.TRISA0
#define PIN20_TRIS TRISAbits.TRISA1
#define PIN21_TRIS TRISBbits.TRISB0
#define PIN22_TRIS TRISBbits.TRISB1
#define PIN23_TRIS TRISBbits.TRISB2
#define PIN24_TRIS TRISBbits.TRISB3
#define PIN25_TRIS TRISCbits.TRISC0
#define PIN26_TRIS TRISCbits.TRISC1
#define PIN27_TRIS TRISCbits.TRISC2
#define PIN30_TRIS TRISAbits.TRISA2
#define PIN31_TRIS TRISAbits.TRISA3
#define PIN32_TRIS TRISAbits.TRISA8
#define PIN33_TRIS TRISBbits.TRISB4
#define PIN34_TRIS TRISAbits.TRISA4
#define PIN35_TRIS TRISAbits.TRISA9
#define PIN36_TRIS TRISCbits.TRISC3
#define PIN37_TRIS TRISCbits.TRISC4
#define PIN38_TRIS TRISCbits.TRISC5
#define PIN41_TRIS TRISBbits.TRISB5
#define PIN43_TRIS TRISBbits.TRISB7
#define PIN44_TRIS TRISBbits.TRISB8

//PIN NUMBER TO LATCH REGISTER (LAT) (EX: PIN2_LAT = ON)
#define PIN1_LAT  LATBbits.LATB9
#define PIN2_LAT  LATCbits.LATC6
#define PIN3_LAT  LATCbits.LATC7
#define PIN4_LAT  LATCbits.LATC8
#define PIN5_LAT  LATCbits.LATC9
#define PIN8_LAT  LATBbits.LATB10
#define PIN9_LAT  LATBbits.LATB11
#define PIN11_LAT LATBbits.LATB13
#define PIN12_LAT LATAbits.LATA10
#define PIN13_LAT LATAbits.LATA7
#define PIN14_LAT LATBbits.LATB14
#define PIN15_LAT LATBbits.LATB15
#define PIN19_LAT LATAbits.LATA0
#define PIN20_LAT LATAbits.LATA1
#define PIN21_LAT LATBbits.LATB0
#define PIN22_LAT LATBbits.LATB1
#define PIN23_LAT LATBbits.LATB2
#define PIN24_LAT LATBbits.LATB3
#define PIN25_LAT LATCbits.LATC0
#define PIN26_LAT LATCbits.LATC1
#define PIN27_LAT LATCbits.LATC2
#define PIN30_LAT LATAbits.LATA2
#define PIN31_LAT LATAbits.LATA3
#define PIN32_LAT LATAbits.LATA8
#define PIN33_LAT LATBbits.LATB4
#define PIN34_LAT LATAbits.LATA4
#define PIN35_LAT LATAbits.LATA9
#define PIN36_LAT LATCbits.LATC3
#define PIN37_LAT LATCbits.LATC4
#define PIN38_LAT LATCbits.LATC5
#define PIN41_LAT LATBbits.LATB5
#define PIN43_LAT LATBbits.LATB7
#define PIN44_LAT LATBbits.LATB8

//PIN NUMBER TO ANALOG REGISTER (AN) (EX: AD1CHSbits.CH0SA = PIN24_AN)
#define PIN11_AN 11
#define PIN14_AN 10
#define PIN15_AN  9
#define PIN19_AN  0
#define PIN20_AN  1
#define PIN21_AN  2
#define PIN22_AN  3
#define PIN23_AN  4
#define PIN24_AN  5
#define PIN25_AN  6
#define PIN26_AN  7
#define PIN27_AN  8
#define PIN36_AN 12

//PIN NUMBER TO REMAPPABLE PIN REGISTER (RP) FOR INPUT (EX: INT1_I = PIN2_RPI)
#define PIN1_RPI   9
#define PIN2_RPI  22
#define PIN3_RPI  23
#define PIN4_RPI  24
#define PIN5_RPI  25
#define PIN8_RPI  10
#define PIN9_RPI  11
#define PIN11_RPI 13
#define PIN14_RPI 14
#define PIN15_RPI 15
#define PIN19_RPI  5
#define PIN20_RPI  6
#define PIN21_RPI  0
#define PIN22_RPI  1
#define PIN23_RPI  2
#define PIN24_RPI  3
#define PIN25_RPI 16
#define PIN26_RPI 17
#define PIN27_RPI 18
#define PIN33_RPI  4
#define PIN36_RPI 19
#define PIN37_RPI 20
#define PIN38_RPI 21
#define PIN43_RPI  7
#define PIN44_RPI  8

//REMAPPABLE PIN INPUT REGISTER FUNCTIONS (RPIR) (EX: INT1_I = PIN2_RPI)
#define INT1_I   RPINR0bits.INT1R
#define INT2_I   RPINR1bits.INT2R
#define IC1_I    RPINR7bits.IC1R
#define IC2_I    RPINR7bits.IC2R
#define IC3_I    RPINR8bits.IC3R
#define IC4_I    RPINR8bits.IC4R
#define IC5_I    RPINR9bits.IC5R
#define OCFA_I   RPINR11bits.OCFAR
#define OCFB_I   RPINR11bits.OCFBR
#define SCK1IN_I RPINR20bits.SCK1R
#define SDI1_I   RPINR20bits.SDI1R
#define SS1IN_I  RPINR21bits.SS1R
#define SCK2IN_I RPINR22bits.SCK2R
#define SDI2_I   RPINR22bits.SDI2R
#define SS2IN_I  RPINR23bits.SS2R
#define T2CK_I   RPINR3bits.T2CKR
#define T3CK_I   RPINR3bits.T3CKR
#define T4CK_I   RPINR4bits.T4CKR
#define T5CK_I   RPINR4bits.T5CKR
#define U1CTS_I  RPINR18bits.U1CTSR
#define U1RX_I   RPINR18bits.U1RXR
#define U2CTS_I  RPINR19bits.U2CTSR
#define U2RX_I   RPINR19bits.U2RXR

//PIN NUMBER TO REMAPPABLE PIN REGISTER (RP) FOR OUTPUT (EX: PIN5_RPO = OC_5)
#define PIN1_RPO  RPOR4bits.RP9R
#define PIN2_RPO  RPOR11bits.RP22R
#define PIN3_RPO  RPOR11bits.RP23R
#define PIN4_RPO  RPOR12bits.RP24R
#define PIN5_RPO  RPOR12bits.RP25R
#define PIN8_RPO  RPOR5bits.RP10R
#define PIN9_RPO  RPOR5bits.RP11R
#define PIN11_RPO RPOR6bits.RP13R
#define PIN14_RPO RPOR7bits.RP14R
#define PIN15_RPO RPOR7bits.RP15R
#define PIN19_RPO RPOR2bits.RP5R
#define PIN20_RPO RPOR3bits.RP6R
#define PIN21_RPO RPOR0bits.RP0R
#define PIN22_RPO RPOR0bits.RP1R
#define PIN23_RPO RPOR1bits.RP2R
#define PIN24_RPO RPOR1bits.RP3R
#define PIN25_RPO RPOR8bits.RP16R
#define PIN26_RPO RPOR8bits.RP17R
#define PIN27_RPO RPOR9bits.RP18R
#define PIN33_RPO RPOR2bits.RP4R
#define PIN36_RPO RPOR9bits.RP19R
#define PIN37_RPO RPOR10bits.RP20R
#define PIN38_RPO RPOR10bits.RP21R
#define PIN43_RPO RPOR3bits.RP7R
#define PIN44_RPO RPOR4bits.RP8R

//REMAPPABLE PIN OUTPUT REGISTER FUNCTIONS (RPOR) (EX: PIN5_RPO = OC_5)
#define NULL_O     0
#define C1OUT_O    1
#define C2OUT_O    2
#define U1TX_O     3
#define U1RTS_O    4
#define U2TX_O     5
#define U2RTS_O    6
#define SDO1_O     7
#define SCK1OUT_O  8
#define SS1OUT_O   9
#define SDO2_O    10
#define SCK2OUT_O 11
#define SS2OUT_O  12
#define OC1_O     18
#define OC2_O     19
#define OC3_O     20
#define OC4_O     21
#define OC5_O     22
#define CTPLS_O   29
#define C3OUT_O   30

//OUTPUT COMPARE TIMER SELECT (OCTSEL)
#define SYSTEM_CLOCK 0b111
#define TIMER1       0b100
#define TIMER2       0b000
#define TIMER3       0b001
#define TIMER4       0b010
#define TIMER5       0b011

//OUTPUT COMPARE MODE
#define CENTER_ALIGNED_PWM                  0b111
#define EDGE_ALIGNED_PWM                    0b110
#define DOUBLE_COMPARE_CONTINUOUS_PULSE     0b101
#define DOUBLE_COMPARE_SINGLE_SHOT          0b100
#define SINGLE_COMPARE_CONTINUOUS_PULSE     0b011
#define SINGLE_COMPARE_SINGLE_SHOT_HIGH_LOW 0b010
#define SINGLE_COMPARE_SINGLE_SHOT_LOW_HIGH 0b001
#define OUTPUT_COMAPRE_DISABLE              0b000

#endif

