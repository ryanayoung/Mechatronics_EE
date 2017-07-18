void initUART(void)
{
	UBRR0H = 0;
	UBRR0L = BAUD_SET;			//defineBaud
	UCSR0B = 0b00001000;		//enable TX
	UCSR0C = 0b00000110;		//8-bit data, 1 stop
}

void txUART(unsigned char charValue)
{
	while(!(UCSR0A & (1<<UDRE0)));	//wait until buffer is empty
	UDR0 = charValue;				//write value to TX
}

void sendString(unsigned char* stringIndex)
{
	while(*stringIndex != 0x00)		//until end of string
	{
		txUART(*stringIndex);
		stringIndex++;
	}
}