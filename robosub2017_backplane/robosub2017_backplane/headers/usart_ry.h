/******************************************************************************
	usart_ry.h
	
	Created: 	12/3/2016 3:36:01 PM
	Author:		Ryan Young
	RedID:		817447547

******************************************************************************/

/******************************************************************************
	USART initialization|
******************************************************************************/
void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
		/*Enable receiver, transmitter, and receive interrupt */
	
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

/******************************************************************************
	USART string transmit|
		parses a string argument and passes each character to the
		USART_Transmit function.
******************************************************************************/
void USART_Transmit_TX(char string[])
{
	int wordsize = strlen(string);
	int counter = 0;
	
	while(wordsize != counter)
	{
		USART_Transmit(string[counter]);
		counter++;
	}
}

/******************************************************************************
	USART CAN FRAME transmit|
		transmits a full CAN frame struct(tCAN) over UART
******************************************************************************/
void USART_CAN_TX(tCAN data)
{
	USART_Transmit(start_byte); //start_byte
	USART_Transmit(data.id >> 3); //CanID_High
	
	//CanID_Low, RTR, Length
	USART_Transmit((data.id << 5) | (data.header.rtr <<4) | data.header.length);
	
	//read back all data received.
	if(!data.header.rtr){
		for (uint8_t t = 0; t < data.header.length;t++) {
			USART_Transmit(data.data[t]); //data
		}
	}
}
