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

#include "Headers/global.h"
#include "Headers/defines.h"
#include "Headers/functions.h"
#include "Headers/spi_ry.h"
//#include "Headers/usart_ry.h"
#include "Headers/mcp2515_ry_def.h"
#include "Headers/mcp2515_ry.h"


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

uint8_t RxID = 0x10;  //M
//uint8_t RxID = 0x20;	//S

uint8_t TxID = 0x20;	//M
//uint8_t TxID = 0x001;	//S
tCAN spi_char;		// message package

int main(void)
{
	setup();
	
	//This is temporary. Reference back to know how to make the frame
	spi_char.id = 0x10;
	spi_char.header.length = 4;
	spi_char.header.rtr = 0;
	spi_char.data[0] = 'F';
	spi_char.data[1] = 'U';
	spi_char.data[2] = 'C';
	spi_char.data[3] = 'FK';
	
	
	
	
	//Percy's Poker Code
	/* 
	Laser_1();
	_delay_ms(1000);
	
	for(int x=0; x<20; x++)
	{
		Laser_2();
		_delay_ms(25);
		Laser_2();
		_delay_ms(145);
	}
	
	Laser_1();*/
	//mcp2515_send_message(&spi_char);
	
	while(1) // Infinite loop while 1 is true.
	{
		 uint8_t test = mcp2515_send_message(&spi_char);
		 if(test == 0x01){
			 //Laser_1();
		 }
	}
}

void Laser_1(){
	PORTD ^= (1<<PORTD4);
}

void Laser_2(){
	PORTD ^= (1<<PORTD3);
}

void Torpedo_1(){
	PORTC ^= (1<<PORTC3);
}

void Torpedo_2(){
	PORTC ^= (1<<PORTC2);
}

void Dropper_1(){
	PORTC ^= (1<<PORTC1);
}

void Dropper_2(){
	PORTC ^= (1<<PORTC0);
}

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

void setup()
{	
	// Set outputs
	DDRB |= (1<<DDB0);	// D8	Claw 4
	DDRB |= (1<<DDB1);	// D9	Claw 3
	
	DDRC |= (1<<DDC0);	// A0	Dropper 2
	DDRC |= (1<<DDC1);	// A1	Dropper 1
	DDRC |= (1<<DDC2);	// A2	Torpedo 2
	DDRC |= (1<<DDC3);	// A3	Torpedo 1
	
	DDRD |= (1<<DDD3);	// D3	Laser 2
	DDRD |= (1<<DDD4);	// D4	Laser 1
	DDRD |= (1<<DDD5);	// D5	Spare
	DDRD |= (1<<DDD6);	// D6	Claw 6
	DDRD |= (1<<DDD7);	// D7	Claw 5
	
	DDRE |= (1<<DDE2);	// A6	Claw 2
	DDRE |= (1<<DDE3);	// A7	Claw 1
	
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
	
	
	
	// Initializes comms with CAN transciever / controller
	mcp2515_init(CANSPEED_500);
}




/*

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

