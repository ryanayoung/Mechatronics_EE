/*
 * WCB.c
 *
 * This project is to create firmware for the 2017 Mechatronics RoboSub Digital / Weapons Control Board or "WCB"
 * 
 * Created: 6/10/2017 1:16:27 PM
 * Author : Andrew Grewe
 */ 
#define F_CPU 16000000UL // 16MHz clock from the debug processor
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>



/******************************************************************************
	CANBUS ID definitions|
		
******************************************************************************/
//RxID is your device ID that you allow messages to receive
#define Rx0ID  0x002
#define Rx1ID  0x002
#define Rx2ID  0x002
#define Rx3ID  0x002
#define Rx4ID  0x002
#define Rx5ID  0x002


/******************************************************************************
	CANBUS header files|
	These must be placed after define RxIDs to prevent errors
******************************************************************************/
#include "Headers/global.h"
#include "Headers/defines.h"
#include "Headers/functions.h"
#include "Headers/spi_ry.h"
// #include "Headers/usart_ry.h"
#include "Headers/mcp2515_ry_def.h"
#include "Headers/mcp2515_ry.h"
#include "headers/can_frames.h"		//CAN frames in tCAN struct format



/******************************************************************************
	Function Prototypes|
		
******************************************************************************/
void setup();
void Laser_1();
void Laser_2();
void Torpedo_1();
void Torpedo_2();
void Dropper_1();
void Dropper_2();
void Claw_1();
void Claw_2();
void Claw_3();
void Claw_4();
void Claw_5();
void Claw_6();
void Spare();
void Poker();



/******************************************************************************
	global variable definitions|
		
******************************************************************************/
tCAN CANRX_buffer;		// message package
volatile uint8_t rx_flag = 0;




int main(void)
{
	setup();
	
	/*
	//This is temporary. Reference back to know how to make the frame
	spi_char.id = 0x10;
	spi_char.header.length = 4;
	spi_char.header.rtr = 0;
	spi_char.data[0] = 'F';
	spi_char.data[1] = 'U';
	spi_char.data[2] = 'C';
	spi_char.data[3] = 'FK';
	*/
	
	// mcp2515_send_message(&spi_char);
	
	while(1) // Infinite loop while 1 is true.
	{
		 if(rx_flag)	// if data received on CAN...
		 {
			 ATOMIC_BLOCK(ATOMIC_RESTORESTATE)	// disables interrupts
			 {
				 // matches received ID.  if current request, returns current data
				 // if more cases are required, will convert to a switch-case
				 if (CANRX_buffer.id == Weapon1_Command.id)
				 {
					 mcp2515_send_message(&Weapon1_Confirm);
					 Laser_1();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon2_Command.id)
				 {
					 mcp2515_send_message(&Weapon2_Confirm);
					 Laser_2();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon3_Command.id)
				 {
					 mcp2515_send_message(&Weapon3_Confirm);
					 Torpedo_1();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon4_Command.id)
				 {
					 mcp2515_send_message(&Weapon4_Confirm);
					 Torpedo_2();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon5_Command.id)
				 {
					 mcp2515_send_message(&Weapon5_Confirm);
					 Dropper_1();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon6_Command.id)
				 {
					 mcp2515_send_message(&Weapon6_Confirm);
					 Dropper_2();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon7_Command.id)
				 {
					 mcp2515_send_message(&Weapon7_Confirm);
					 Claw_1();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon8_Command.id)
				 {
					 mcp2515_send_message(&Weapon8_Confirm);
					 Claw_2();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon9_Command.id)
				 {
					 mcp2515_send_message(&Weapon9_Confirm);
					 Claw_3();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon10_Command.id)
				 {
					 mcp2515_send_message(&Weapon10_Confirm);
					 Claw_4();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon11_Command.id)
				 {
					 mcp2515_send_message(&Weapon11_Confirm);
					 Claw_5();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon12_Command.id)
				 {
					 mcp2515_send_message(&Weapon12_Confirm);
					 Claw_6();
					 //send over can
				 }
				 else if (CANRX_buffer.id == Weapon13_Command.id)
				 {
					 mcp2515_send_message(&Weapon13_Confirm);
					 Spare();
					 //send over can
				 }
				 rx_flag = 0;	// clear receive flag
			 }	// end ATOMIC_BLOCK
		 }
	}
}	// end main



void setup()
{
	// Set outputs
	DDRB |= (1<<DDB0);	// D8 -> PB0	Claw 4
	DDRB |= (1<<DDB1);	// D9 -> PB1	Claw 3
	
	DDRC |= (1<<DDC0);	// A0 -> PC0	Dropper 2
	DDRC |= (1<<DDC1);	// A1 -> PC1	Dropper 1
	DDRC |= (1<<DDC2);	// A2 -> PC2	Torpedo 2
	DDRC |= (1<<DDC3);	// A3 -> PC3	Torpedo 1
	
	DDRD |= (1<<DDD3);	// D3 -> PD3	Laser 2
	DDRD |= (1<<DDD4);	// D4 -> PD4	Laser 1
	DDRD |= (1<<DDD5);	// D5 -> PD5	Spare
	DDRD |= (1<<DDD6);	// D6 -> PD6	Claw 6
	DDRD |= (1<<DDD7);	// D7 -> PD7	Claw 5
	
	DDRE |= (1<<DDE2);	// A6 -> PE2	Claw 2
	DDRE |= (1<<DDE3);	// A7 -> PE3	Claw 1
	
	// Ensure all outputs are off
	PORTB &= ~(1<<PORTB0);
	PORTB &= ~(1<<PORTB1);
	
	PORTC &= ~(1<<PORTC0);
	PORTC &= ~(1<<PORTC1);
	PORTC &= ~(1<<PORTC2);
	PORTC &= ~(1<<PORTC3);
	
	PORTD &= ~(1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
	PORTD &= ~(1<<PORTD5);
	PORTD &= ~(1<<PORTD6);
	PORTD &= ~(1<<PORTD7);
	
	PORTE &= ~(1<<PORTE2);
	PORTE &= ~(1<<PORTE3);
	
	// CAN bus stuff
	GPIO_init();
	INTERRUPT_init();
	SPI_masterInit();
	mcp2515_init(CANSPEED_500);	// Initializes communications with CAN transciever / controller
}

// Lasers will toggle each time the function is called
void Laser_1(){
	PORTD ^= (1<<PORTD4);
}

void Laser_2(){
	PORTD ^= (1<<PORTD3);
}

// Each Weapon device will toggle on for 50mS and then toggle off
void Torpedo_1(){
	PORTC ^= (1<<PORTC3);
	_delay_ms(50);
	PORTC &= ~(1<<PORTC3);
}

void Torpedo_2(){
	PORTC ^= (1<<PORTC2);
	_delay_ms(50);
	PORTC &= ~(1<<PORTC2);
}

void Dropper_1(){
	PORTC ^= (1<<PORTC1);
	_delay_ms(50);
	PORTC &= ~(1<<PORTC1);
}

void Dropper_2(){
	PORTC ^= (1<<PORTC0);
	_delay_ms(50);
	PORTC &= ~(1<<PORTC0);
}

// Each claw device will be triggered each time the function is called
void Claw_1(){
	PORTE ^= (1<<PORTE3);
}

void Claw_2(){
	PORTE ^= (1<<PORTE2);
}

void Claw_3(){
	PORTB ^= (1<<PORTB1);
}

void Claw_4(){
	PORTB ^= (1<<PORTB0);
}

void Claw_5(){
	PORTD ^= (1<<PORTD7);
}

void Claw_6(){
	PORTD ^= (1<<PORTD6);
}

void Spare(){
	PORTD ^= (1<<PORTD5);
}



/******************************************************************************
	Custom weapon functions|

******************************************************************************/
void Poker()
{
	// Raise the poker
	Claw_1();
	_delay_ms(1000);
	
	// Spam the poker
	for(int x=0; x<20; x++)
	{
		Claw_2();
		_delay_ms(25);
		Claw_2();
		_delay_ms(145);
	}
	
	// Lower the poker
	Claw_1();
}


/******************************************************************************
	CAN RECEIVE interrupt on pin PD2|
******************************************************************************/
ISR(INT0_vect)
{
	mcp2515_get_message(&CANRX_buffer);	// get canbus message
	rx_flag = 1;  //set flag
}











/* // LED testing code 

Laser_1();	// turn on
Laser_2();
Torpedo_1();
Torpedo_2();
Dropper_1();
Dropper_2();
Claw_1();
Claw_2();
Claw_3();
Claw_4();
Claw_5();
Claw_6();
Spare();
_delay_ms(200);
Laser_1();	// turn off
Laser_2();
Torpedo_1();
Torpedo_2();
Dropper_1();
Dropper_2();
Claw_1();
Claw_2();
Claw_3();
Claw_4();
Claw_5();
Claw_6();
Spare();
_delay_ms(600);
Laser_1();	// turn on
Laser_2();
Torpedo_1();
Torpedo_2();
Dropper_1();
Dropper_2();
Claw_1();
Claw_2();
Claw_3();
Claw_4();
Claw_5();
Claw_6();
Spare();
_delay_ms(600);
Laser_1();	// turn off
Laser_2();
Torpedo_1();
Torpedo_2();
Dropper_1();
Dropper_2();
Claw_1();
Claw_2();
Claw_3();
Claw_4();
Claw_5();
Claw_6();
Spare();
_delay_ms(2000);

*/

