/*
 * SIB.h
 *
 * Created: 6/21/2017 11:10:43 PM
 *  Author: Scott Szafranski
 */ 


#ifndef SIB_H_
#define SIB_H_

#define BAUD 9600
#define BAUD_SET (((F_CPU / (BAUD * 16UL))) - 1)	//declare long since int is 16bit


#define STATE_PERIOD 16.7
#define OUTPUT 1
#define INPUT 0

//if DD set input, define PORTxn as
#define PULL_UP_ON 1
#define PULL_UP_OFF 0	//HIGH Z

//if DD set output, define PORTxn as
#define HIGH 1
#define LOW 0
#define LED_ON PORTB|= (HIGH<<PORTB1)
#define LED_OFF PORTB &= 0b11111101
#define LED_TOGGLE PINB|= (HIGH<<PORTB1)

//R/T i2c, master mode only
#define START 0x08
#define REP_START 0x10
#define ARB 0x38

//transmit i2c
#define T_ADD_ACK 0x18
#define T_ADD_NACK 0x20
#define T_ACK 0x28
#define T_NACK 0x30

//receive i2c
#define R_ADD_ACK 0x40
#define R_ADD_NACK 0x48
#define R_ACK 0x50
#define R_NACK 0x58

#define LAST_STATE 6

#define LEAK 1
#define DEPTH 2 
#define GENERAL 3

//i2c control register
#define CLEAR_TWINT TWCR0 &= 0b01111111;
#define CLEAR_TWEA TWCR0 &= 0b10111111;
#define CLEAR_TWSTA TWCR0 &= 0b11011111;
#define CLEAR_TWSTO TWCR0 &= 0b11101111;
#define CLEAR_TWWC TWCR0 &= 0b11110111;
#define CLEAR_TWEN TWCR0 &= 0b11111011;
#define CLEAR_TWIE TWCR0 &= 0b11111110;

#define SET_TWINT TWCR0 |= 0b10000000;
#define SET_TWEA TWCR0 |= 0b01000000;
#define SET_TWSTA TWCR0 |= 0b00100000;
#define SET_TWSTO TWCR0 |= 0b00010000;
#define SET_TWWC TWCR0 |= 0b00001000;
#define SET_TWEN TWCR0 |= 0b00000100;
#define SET_TWIE TWCR0 |= 0b00000001;

#define I2C_NEXT TWCR0 = (0<<TWSTA) | (0<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE)
#define I2C_STOP TWCR0 = (0<<TWSTA) | (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE)
#define I2C_START TWCR0 =  (1<<TWSTA) | (0<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE)
#define I2C_ACK TWCR0 =  (0<<TWSTA) | (0<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);


void i2c_FSM(void);
void init_i2c(void);
//void init_temp(void);
void init_int_press(void);
uint8_t i2c_read(uint8_t, uint8_t);
uint8_t i2c_write(uint8_t, uint8_t);

void load_i2c_data(void);
//void get_ext_press(void);

void init_state_timer(void);
void initUART(void);
void txUART(unsigned char data);
void sendString(unsigned char* stringIndex);

void CAN_HANDLE(void);
void it_CAN(void);
void ep_CAN(void);
void ip_CAN(void);

void debug_can_tx(void);
void debug_state(void);

void int_p1(void);
void int_p2(void);
void int_p3(void);

void int_t1(void);
void int_t2(void);
void int_t3(void);

void update_ep(void);

void error_TCA(uint8_t);
void error_MPLW(uint8_t);
void error_MPLR(uint8_t);
void error_LM92(uint8_t);

void sys_config(void)
{
	DDRB|=(OUTPUT<<DDB1);	//LED, pin 13

	DDRC|=(INPUT<<DDC1);	//T1, pin 24
	DDRC|=(INPUT<<DDC2);	//T2, pin 25
	DDRC|=(INPUT<<DDC3);	//T3, pin 26

	DDRD|=(INPUT<<DDD0);	//RX, pin 30
	DDRD|=(OUTPUT<<DDD1);	//TX, pin 31
	
	DDRD|=(INPUT<<DDD2);	//CAN INT, pin 32

	DDRD|=(INPUT<<DDD3);	//TCRIT1, pin 1
	DDRD|=(INPUT<<DDD4);	//TCRIT2, pin 2
	DDRD|=(INPUT<<DDC5);	//TCRIT3, pin 9
	DDRD|=(INPUT<<DDD6);	//P1, pin 10
	DDRD|=(INPUT<<DDD7);	//P2, pin 11
	DDRB|=(INPUT<<DDB0);	//P3, pin 12

	//ADC: ADC6/ADC7/PC0
	//UART: RX-PD0, TX-PD1
	//SPI: MOSI - PB3, MISO - PB4, SCK - PB5
	DDRB|=(OUTPUT<<DDB3);	//Slave Select
	PORTB|= (HIGH<<PORTB3);	//Default Value: High

	DDRC |= (OUTPUT<<DDC4);
	PORTC |= (HIGH<<PORTC4);

	//RESET: PC6
}


uint32_t getMax(uint32_t num1, uint32_t num2, uint32_t num3)
{
	uint32_t temp;
	if (num1 > num2)
	{
		temp = num1;
	}
	else
	{
		temp = num2;
	}

	if (temp > num3)
	{
		return temp;
	}
	else
	{
		return num3;
	}
}

uint32_t getMin(uint32_t num1, uint32_t num2, uint32_t num3)
{
	uint32_t temp;
	if (num1 < num2)
	{
		temp = num1;
	}
	else
	{
		temp = num2;
	}

	if (temp < num3)
	{
		return temp;
	}
	else
	{
		return num3;
	}
}

uint32_t getMedian_of3(uint32_t num1, uint32_t num2, uint32_t num3)
{
	uint32_t max, min;
	
	max = getMax(num1,num2,num3);
	min = getMin(num1,num2,num3);

	if ((num1 != max) && (num1 != min))
	{
		return num1;
	}
	else if ((num2 != max) && (num2 != min))
	{
		return num2;
	}
	else
	{
		return num3;
	}
}



#endif /* SIB_H_ */