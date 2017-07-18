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
	
	//set output for status LED on PB0
	SET_OUTPUT(LED2);
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