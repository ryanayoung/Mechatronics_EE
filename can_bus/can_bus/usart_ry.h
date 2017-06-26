/*
 * usart_ry.h
 *
 * Created: 12/3/2016 3:36:01 PM
 *  Author: grimj
 */ 


/******************************************************************************
	USART initialization|
******************************************************************************/
void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
		/*Enable receiver and transmitter */
	
	UCSR0C |= (3<<UCSZ00);
		//(3<<UCSZ00) shifts 0b11 left into the UCSZ[1:0] position
		//	to enable an 8-bit character size
}

/******************************************************************************
	USART receive function|
		currently not used
******************************************************************************/
uint8_t USART_Receive( void )
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}

/******************************************************************************
	USART transmit function|
		transmits a character across the uart tx/rx pins
******************************************************************************/
void USART_Transmit( uint8_t data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}