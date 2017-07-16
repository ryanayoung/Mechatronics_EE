/******************************************************************************
	functions.h
	
	Created: 	12/3/2016 2:19:19 PM
	Author:		Ryan Young
	RedID:		817447547

******************************************************************************/

/******************************************************************************
	GPIO initiation|
		enabling inputs, outputs, and pull-up resistors
******************************************************************************/
void GPIO_init(void)
{
	//SPI GPIO set in spi_ry.h
	
	//set input for INT line PD2
	SET_INPUT(INT);
	
	//set output for status LEDs
	SET_OUTPUT(LED1);
	SET_OUTPUT(LED2);
	SET_OUTPUT(LED3);
	SET_OUTPUT(LED4);
	
	
	
	
	//set inputs for current sense
	SET_INPUT(V24_pin);
	SET_INPUT(V6_pin);
	SET_INPUT(V5_pin);
	
	
}

/******************************************************************************
	interrupt initiation|
******************************************************************************/
void INTERRUPT_init(void)
{
	//enable external interrupt for INT line from mcp2515
	EIMSK |= (1<<INT0);//enable
	EICRA &= ~(3<<ISC00);//low level interrupt for INT0
	
	sei(); //global interrupt enable
}


/******************************************************************************
	ADC initiation|
******************************************************************************/

void ADC_init(void) 
{ 
	 ADMUX |= (1<<REFS0); //Vcc reference, ADC0 start by default
	 ADCSRA |= (1<<ADEN) | (1<<ADLAR) | (1<<ADIE); 
	 //enables the ADC, enables the auto trigger, enables ADC interrupts 
 
	 //ADCSRB |= (1<<ADTS1) | (1<<ADTS0); //set auto trigger for Timer0 match A. 
} 
 
/****************************************************************************** 
	8 bit Timer setup| 
		not currently used.
******************************************************************************/ 
/* void b8_Timer_init(void) 
{ 
	 TCCR0A |= (1 << WGM01); //set to CTC mode 
	 TCCR0B |= (1<<CS02) | (1<<CS00); //1024 prescaler 
	 TIMSK0 |= (1<<OCIE0A); //enable compare interrupt 
	 OCR0A = 39;//roughly 5ms or 200Hz. 
} */

/****************************************************************************** 
 map function| map(value, fromLow, fromHigh, toLow, toHigh) 
 maps a range of values to another range. 
 I've used this function in a mechatronics project using a potentiometer 
 to control the PWM signal to an ESC controlling a thruster. 
 same function used by the arduino library 
 https://www.arduino.cc/en/Reference/Map 
******************************************************************************/ 
long map(long x, long in_min, long in_max, long out_min, long out_max) 
{ 
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; 
}