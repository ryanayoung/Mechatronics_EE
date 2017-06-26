

void mcp2515_write_register( uint8_t adress, uint8_t data )
{
	SET_L(SS);//enable slave
	
	SPI_txrx(CAN_WRITE);//send write instruction
	SPI_txrx(adress);//send address
	SPI_txrx(data);//send value
	
	SET_H(SS);//disable slave
}