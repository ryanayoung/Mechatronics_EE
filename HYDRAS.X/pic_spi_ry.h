/*
description! coming soon
*/

#ifndef PIC_SPI_RY
#define PIC_SPI_RY

//please test
//in progress, needs commenting, basically done
void SPI_masterInit(void)
{
  //set SS, MOSI, SCK TO OUTPUT
  SET_OUTPUT(SS);
  TRISCbits.TRISC0 = 1;
  SET_OUTPUT(SCK);

  //SS high, MIGHT NOT BE NEEDED?
  /*SET_H(SS);
  //MOSI AND SCK low
  SET_L(MOSI);
  SET_L(SCK);*/
  //sets pin 37 to serial data output
  //_RPO(10,20,7);
  //sets pin 38 to serial clock output
  //_RPO(10,21,8);
  RPINR20bits.SDI1R = 22; //sets pin 2 to MISO function
  IFS0bits.SPI1IF = 0;
  IEC0bits.SPI1IE = 0;
  SPI1STATbits.SPIEN = 0;

  SPI1CON1bits.DISSCK = 0;
  SPI1CON1bits.DISSDO = 0;
  SPI1CON1bits.MODE16 = 0;
  SPI1CON1bits.MSTEN = 1;
  SPI1CON1bits.SMP = 0;
  SPI1CON1bits.CKE = 0;
  SPI1CON1bits.CKP = 0;

  SPI1CON1bits.SPRE2 = 0;
  SPI1CON1bits.SPRE1 = 0;
  SPI1CON1bits.SPRE0 = 0;
  SPI1CON1bits.PPRE1 = 1;
  SPI1CON1bits.PPRE0 = 0;

  SPI1STATbits.SPIEN = 1;
}
//needs testing
char SPI_txrx(char val)
{
  while(SPI1STATbits.SPITBF);
      SPI1BUF = val;
  return(SPI1BUF);


}


#endif