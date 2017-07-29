void USART_Init(unsigned int ubrr){
  SET_OUTPUT(TX);
  SET_INPUT(RX);
  U1BRG = FCY / 16 / UART_BAUD - 1;
  U1MODEbits.UARTEN = 1;
  U1STAbits.UTXEN = 1;
  //pin 4 tx
  RPOR12bits.RP24R = 3;
  //pin 5 rx
  RPINR18bits.U1RXR1 = 25;
}




uint8_t USART_Receive( void ){
  while(U1STAbits.UTXBF == 1);
  return U1TXREG;
}

void USART_Transmit( uint8_t data ){
  while(U1STAbits.UTXBF == 1);
  U1TXREG = data;
}

/******************************************************************************
	USART CAN FRAME transmit|
		transmits a full CAN frame struct(tCAN) over UART
******************************************************************************/
void USART_CAN_TX(struct tCAN data){
    
	//USART_Transmit(start_byte); //start_byte
	USART_Transmit(data.id >> 3); //CanID_High
	
	//CanID_Low, RTR, Length
	USART_Transmit((data.id << 5) | (data.header.rtr <<4) | data.header.length);
	
    uint8_t t;
	//read back all data received.
	if(!data.header.rtr){
		for (t = 0; t < data.header.length;t++) {
			USART_Transmit(data.data[t]); //data
		}
	}
}
