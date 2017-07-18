void USART_Init(unsigned int ubrr)
{

  U1BRG = FCY / 16 / UART_BAUD - 1;
  U1MODEbits.UARTEN = 1;
  U1STAbits.UTXEN = 1;
  //pin 1 tx
  RPOR12bits.RP24R = 3;
  //pin 2 rx
  RPOR11bits.U1RXR = 5;
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
