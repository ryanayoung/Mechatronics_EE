/*
 * mcp2515_ry_def.h
 *
 * Created: 12/3/2016 2:43:20 PM
 *  Author: grimj
 */ 


//from the MCP2515 DATA SHEET SPI instruction set
//TABLE 12-1
#define CAN_RESET			0xC0
#define	CAN_READ			0x03
#define	CAN_READ_RX_BUFF	0x90
#define	CAN_WRITE			0x02
#define	CAN_LOAD_TX_BUFF	0x40
#define	CAN_RTS				0x80
#define CAN_READ_STATUS		0xA0
#define	CAN_RX_STATUS		0xB0
#define	CAN_BIT_MODIFY		0x05
//send this three words, one of these, then an address, then a value