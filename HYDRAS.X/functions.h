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

	//set input for INT line RB14
	SET_INPUT(INT);

	//set output for status LED on PB0
	SET_OUTPUT(LED2);
}

/******************************************************************************
	interrupt initiation|
******************************************************************************/
void INTERRUPT_init(void)
{
    //pg 106
    //set a pin to be external interrupt
		//set pin 37, rp20 to external interrupt
    RPINR0bits.INT1R = 20;
    INTCON1.NSTDIS = 1;
	//EIMSK |= (1<<INT0);//enable
    IPC5.INT1IP = 1;
		IEC1.INT1IE = 1;
		INTCON1.INT1EP = 1;
	//EICRA &= ~(3<<ISC00);//low level interrupt for INT0

	//sei(); //global interrupt enable
}
