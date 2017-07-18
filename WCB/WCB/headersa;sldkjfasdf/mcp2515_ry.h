/* Copyright (c) 2007 Fabian Greif
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------

/******************************************************************************
RYAN YOUNG
	This header file was acquired from the SPARKFUN can-bus arduino library.
	It has been HEAVILY modified for clarity and use in my own code,
	as well as most of the original comments were in German.

	location of SPARKFUN library:
	https://github.com/sparkfun/SparkFun_CAN-Bus_Arduino_Library
******************************************************************************/


/******************************************************************************
	tCAN|
		this is a structure to hold a canbus message frame.
		It contains:
			the message ID
			if it's a "remote transmit receive" frame
			the length: from 0 to 8 bytes(0 if rtr is 1)
			the 8 bytes
******************************************************************************/
typedef struct
{
	uint16_t id;
	struct {
		int8_t rtr : 1;
		uint8_t length : 4;
	} header;
	uint8_t data[8];
} tCAN;


void mcp2515_write_register( uint8_t adress, uint8_t data )
{
	SET_L(SS);//enable slave

	SPI_txrx(CAN_WRITE);//send write instruction
	SPI_txrx(adress);//send address
	SPI_txrx(data);//send value

	SET_H(SS);//disable slave
}

uint8_t mcp2515_read_register(uint8_t adress)
{
	uint8_t data;

	SET_L(SS);//enable slave

	SPI_txrx(CAN_READ);
	SPI_txrx(adress);

	data = SPI_txrx(0xff);

	SET_H(SS);//disable slave

	return data;
}

// -------------------------------------------------------------------------
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
	SET_L(SS);

	SPI_txrx(CAN_BIT_MODIFY);
	SPI_txrx(adress);
	SPI_txrx(mask);
	SPI_txrx(data);

	SET_H(SS);
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_read_status(uint8_t type)
{
	uint8_t data;

	SET_L(SS);

	SPI_txrx(type);
	data = SPI_txrx(0xff);

	SET_H(SS);

	return data;
}


/******************************************************************************
	MCP2515 initialization|
		sets up speed, initial conditions, interrupts, GPIO, 
			and receive filters for the canbus transceiver.
		
		If error, PB0 is set high to turn on an error LED.
******************************************************************************/
uint8_t mcp2515_init(uint8_t speed)
{

	// resets MCP2515 and puts it into configuration mode.
	SET_L(SS);
	SPI_txrx(CAN_RESET);
	SET_H(SS);

	//I had to increase this from 10 to 20, as otherwise it would fail.
	//		The MCP2515 needed more time to reset.
	_delay_us(20);

	// load CNF1..3 Register
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(CNF3);  //address 0x28

	SPI_txrx((1<<PHSEG21));		// Bitrate 500 kbps at 16 MHz

	/*after tx the MCP2515 increments the address automatically, so you can
		continue to write into adjacent registers, so the next write goes into
		0x29, the location of CNF2*/
	SPI_txrx((1<<BTLMODE)|(1<<PHSEG11));//CFN2
    	SPI_txrx(speed);//writes to BRPn of CNF1

	// activate interrupts
	SPI_txrx((1<<RX1IE)|(1<<RX0IE));
	SET_H(SS);

	// test if we could read back the value => is the chip accessible?
	if (mcp2515_read_register(CNF1) != speed) {
		SET_H(LED2);

		return false;
	}

	// deactivate the RXnBF Pins (High Impedance State)
	mcp2515_write_register(BFPCTRL, 0);

	// set TXnRTS as inputs
	mcp2515_write_register(TXRTSCTRL, 0);

	
	// turn off filters => receive any message
	mcp2515_write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0));
	mcp2515_write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));


/******************************************************************************
	CANBUS ID|
		Most of these have adjacent registers so we can address the register
		pairs in a single write session.
		
		The original code didn't include this as it was setup to receive all
		messages on the bus.
******************************************************************************/
/*  filters currently disabled.
	
	//enable filtering
	mcp2515_write_register(RXB0CTRL, (1<<RXM0));//buffer0
	mcp2515_write_register(RXB1CTRL, (1<<RXM0));//buffer1

	//Receive Masking:
	//block all ID's other than the exact RxID for buffer0
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXM0SIDH);
	SPI_txrx(0xFF);
	SPI_txrx(0xE0);
	SET_H(SS);
// 	mcp2515_write_register(RXM0SIDH, 0xFF);
// 	mcp2515_write_register(RXM0SIDL, 0xE0);
	//block all ID's other than the exact RxID for buffer1
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXM1SIDH);
	SPI_txrx(0xFF);
	SPI_txrx(0xE0);
	SET_H(SS);
// 	mcp2515_write_register(RXM1SIDH, 0xFF);
// 	mcp2515_write_register(RXM1SIDL, 0xE0);

	//Set RxID
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXF0SIDH);
	SPI_txrx(RxIDHi);
	SPI_txrx(RxIDLow);
	SET_H(SS);
// 	mcp2515_write_register(RXF0SIDH, RxIDHi);//filter0
// 	mcp2515_write_register(RXF0SIDL, RxIDLow);

	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXF1SIDH);
	SPI_txrx(RxIDHi);
	SPI_txrx(RxIDLow);
	SET_H(SS);
// 	mcp2515_write_register(RXF1SIDH, RxIDHi);//filter1
// 	mcp2515_write_register(RXF1SIDL, RxIDLow);
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXF2SIDH);
	SPI_txrx(RxIDHi);
	SPI_txrx(RxIDLow);
	SET_H(SS);
// 	mcp2515_write_register(RXF2SIDH, RxIDHi);//filter2
// 	mcp2515_write_register(RXF2SIDL, RxIDLow);
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXF3SIDH);
	SPI_txrx(RxIDHi);
	SPI_txrx(RxIDLow);
	SET_H(SS);
// 	mcp2515_write_register(RXF3SIDH, RxIDHi);//filter3
// 	mcp2515_write_register(RXF3SIDL, RxIDLow);
	SET_L(SS);
	SPI_txrx(CAN_WRITE);
	SPI_txrx(RXF4SIDH);
	SPI_txrx(RxIDHi);
	SPI_txrx(RxIDLow);
	SET_H(SS);
// 	mcp2515_write_register(RXF4SIDH, RxIDHi);//filter4
// 	mcp2515_write_register(RXF4SIDL, RxIDLow);
*/
/******************************************************************************/

	// reset device to normal mode
	mcp2515_write_register(CANCTRL, 0);
	SET_L(LED2);
	return true;
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_get_message(tCAN *message)
{
	// read status
	uint8_t status = mcp2515_read_status(CAN_RX_STATUS);
	uint8_t addr;
	uint8_t t;
	if (bit_is_set(status,6)) {
		// message in buffer 0
		addr = CAN_READ_RX_BUFF;
	}
	else if (bit_is_set(status,7)) {
		// message in buffer 1
		addr = CAN_READ_RX_BUFF | 0x04;
	}
	else {
		// Error: no message available
		return 0;
	}

	SET_L(SS);
	SPI_txrx(addr);

	// read id
	message->id  = (uint16_t) SPI_txrx(0xff) << 3;
	message->id |=            SPI_txrx(0xff) >> 5;

	SPI_txrx(0xff);
	SPI_txrx(0xff);

	// read DLC
	uint8_t length = SPI_txrx(0xff) & 0x0f;

	message->header.length = length;
	message->header.rtr = (bit_is_set(status, 3)) ? 1 : 0;

	// read data
	for (t=0;t<length;t++) {
		message->data[t] = SPI_txrx(0xff);
	}
	SET_H(SS);

	// clear interrupt flag
	if (bit_is_set(status, 6)) {
		mcp2515_bit_modify(CANINTF, (1<<RX0IF), 0);
	}
	else {
		mcp2515_bit_modify(CANINTF, (1<<RX1IF), 0);
	}

	return (status & 0x07) + 1;
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_send_message(tCAN *message)
{
	uint8_t status = mcp2515_read_status(CAN_READ_STATUS);
	
	

	/*  status info from data sheet:
	  Bit	Function
	   2	TXB0CNTRL.TXREQ
	   4	TXB1CNTRL.TXREQ
	   6	TXB2CNTRL.TXREQ
	 */
	uint8_t address;
	uint8_t t;

	if (bit_is_clear(status, 2)) {
		address = 0x00;
	}
	else if (bit_is_clear(status, 4)) {
		address = 0x02;
	}
	else if (bit_is_clear(status, 6)) {
		address = 0x04;
	}
	else {
		// all buffer used => could not send message
		return 0;
	}

	SET_L(SS);
	SPI_txrx(CAN_LOAD_TX_BUFF | address);

	//split 11bit ID into it's respective register positions
	SPI_txrx(message->id >> 3);
    SPI_txrx(message->id << 5);

	SPI_txrx(0);
	SPI_txrx(0);

	uint8_t length = message->header.length & 0x0f;
	

	if (message->header.rtr) {
		// a rtr-frame has a length, but contains no data
		SPI_txrx((1<<RTR) | length);
	}
	else {
		// set message length
		SPI_txrx(length);

		// data
		for (t=0;t<length;t++) {
			SPI_txrx(message->data[t]);
		}
	}
	SET_H(SS);
	
	
	//Wait for message to "settle" in register
	_delay_us(1);

	// send message
	SET_L(SS);
	address = (address == 0) ? 1 : address;
	SPI_txrx(CAN_RTS | address);
	SET_H(SS);
	

	return address;
}
