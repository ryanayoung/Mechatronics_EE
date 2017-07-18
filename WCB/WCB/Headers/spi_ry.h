/******************************************************************************
	spi_ry.h
	
	Created: 	12/3/2016 3:54:03 PM
	Author:		Ryan Young
	RedID:		817447547

******************************************************************************/
void SPI_masterInit(void)
{
	//set SS, MOSI, & SCK OUTPUT
	SET_OUTPUT(SS);
	SET_OUTPUT(MOSI);
	SET_OUTPUT(SCK);
	
	//SS high
	SET_H(SS);
	// MOSI & SCK low
	SET_L(MOSI);
	SET_L(SCK);
	
	//enable SPI
	SPCR0 |= (1<<SPE) | (1<<MSTR);
	
	//set SCK divider to f_osc/8
	SPCR0 |= 0b11; //sets divider to f_osc/16
	//SPSR |= 1; //f_osc * 2, results in f_osc/8
}

char SPI_txrx(char val)
{
	SPDR0 = val; //send value to buffer
	while(!(SPSR0 & (1<<SPIF))); //wait until complete
	_delay_us(50);
	return(SPDR0);  //return received value
}