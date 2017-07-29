void USART_Init(unsigned int ubrr)
{
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




uint8_t USART_Receive( void )
{
  while(U1STAbits.UTXBF == 1);
  return U1TXREG;
}

void USART_Transmit( uint8_t data )
{
  while(U1STAbits.UTXBF == 1);
  U1TXREG = data;
}
