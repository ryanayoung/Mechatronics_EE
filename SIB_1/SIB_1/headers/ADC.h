void init_ADC(void)
{
	//AREF, Right Adjusted, ADC0
	ADMUX = 0b00000000;

	//AREF, Right Adjusted, ADC6
	//ADMUX = 0b00000110;

	//AREF, Right Adjusted, ADC7
	//ADMUX = 0b00000111;

	//ADCEN, Start conversion, 64 prescaler
	ADCSRA = 0b11001110;

	//Disable digital in on ADC pins
	DIDR0 = 0b11000001;
}

void init_ADC_timer(void)
{
	OCR0A = 77;		//for 400Hz, prescaler at 256
	TCCR0A |= (1<<WGM01);
	TCCR0B = 0b00000100;
	TIMSK0 |= (1<<OCIE0A);
}