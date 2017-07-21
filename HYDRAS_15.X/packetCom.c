// Includes
#include "packetCom.h"
#include "circularBuffer.h"
#include "packets.h"
#include "crc.h"
#include <xc.h>
#include <stdlib.h>
#include "system.h"
#include "types.h"
#include "austins_PIC24FJ32GB004_lib.h"

// Prototypes
static void PWMConfig();

// Variables
static PacketComState CurrentPacketComState;
static uint8 CurrentPacket[MAX_PACKET_BYTECOUNT] = {0};
static Packet* const DataPacket = (Packet*)&CurrentPacket[0];
static volatile uint8 CurrentPacketReadCount;

void PacketComState_Run(void)
{
    /*BYTECOUNT GET*/
    if (PACKETSTATE_READBYTECOUNT == CurrentPacketComState)
    {
	if (CircularBuffer_Read(ReceiveBuffer, &CurrentPacket[0]))
	{
	    if (CurrentPacket[0] > MAX_PACKET_BYTECOUNT || CurrentPacket[0] < MIN_PACKET_BYTECOUNT)
	    {
		return;
	    }

            CurrentPacketReadCount = 2;
	    CurrentPacketComState = PACKETSTATE_READFRAMEID;
	}
    }
    /*FRAMEID GET*/
    else if (PACKETSTATE_READFRAMEID == CurrentPacketComState)
    {
	if (CircularBuffer_Read(ReceiveBuffer, &CurrentPacket[1]))
	{
	    CurrentPacketComState = PacketComState_ParseFrameId();
	}
    }
    /*PINGER HEADING 1 GET*/
    else if (PACKETSTATE_PARSEHEADING1GET == CurrentPacketComState)
    {
        CurrentPacketReadCount += CircularBuffer_ReadCopy(ReceiveBuffer,
                &CurrentPacket[CurrentPacketReadCount],
                CurrentPacket[0] - CurrentPacketReadCount);
	if (CurrentPacketReadCount == CurrentPacket[0])
	{
	    if (!PacketComState_CheckCRC())
	    {
		CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
		return;
	    }

	    DataPacket->Heading1Get.Header.ByteCount = 11;
	    DataPacket->Heading1Get.Heading1 = Heading1;
            DataPacket->Heading1Get.AOI1 = AOI1;
            DataPacket->Heading1Get.Confidence1 = Confidence1;
	    DataPacket->Heading1Get.Crc = crcFast((uint8*)DataPacket, 7);
	    CircularBuffer_Write(TransmitBuffer, &CurrentPacket[0], 11);

	    CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
	    if (U2STAbits.TRMT == 1)
            {
                IFS1bits.U2TXIF = 1;
            }
	}
    }
    /*PINGER HEADING 2 GET*/
    else if (PACKETSTATE_PARSEHEADING2GET == CurrentPacketComState)
    {
        CurrentPacketReadCount += CircularBuffer_ReadCopy(ReceiveBuffer,
                &CurrentPacket[CurrentPacketReadCount],
                CurrentPacket[0] - CurrentPacketReadCount);
	if (CurrentPacketReadCount == CurrentPacket[0])
	{
	    if (!PacketComState_CheckCRC())
	    {
		CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
		return;
	    }

	    DataPacket->Heading2Get.Header.ByteCount = 11;
            DataPacket->Heading2Get.Heading2 = Heading2;
            DataPacket->Heading2Get.AOI2 = AOI2;
            DataPacket->Heading2Get.Confidence2 = Confidence2;
	    DataPacket->Heading2Get.Crc = crcFast((uint8*)DataPacket, 7); // might need to change the number 4 to something else
	    CircularBuffer_Write(TransmitBuffer, &CurrentPacket[0], 11); // need to make sure 11 is the right number to use

	    CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
	    if (U2STAbits.TRMT == 1)
            {
                IFS1bits.U2TXIF = 1;
            }
	}
    }
    /*HYDROPHONES ADC GET*/
    else if (PACKETSTATE_PARSEHYDROPHONESGET == CurrentPacketComState)
    {
        CurrentPacketReadCount += CircularBuffer_ReadCopy(ReceiveBuffer,
                &CurrentPacket[CurrentPacketReadCount],
                CurrentPacket[0] - CurrentPacketReadCount);
	if (CurrentPacketReadCount == CurrentPacket[0])
	{
	    if (!PacketComState_CheckCRC())
	    {
		CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
		return;
	    }

	    DataPacket->HydrophonesGet.Header.ByteCount = 14;
            DataPacket->HydrophonesGet.H1 = H1;
            DataPacket->HydrophonesGet.H2 = H2;
            DataPacket->HydrophonesGet.H3 = H3;
            DataPacket->HydrophonesGet.H4 = H4;
	    DataPacket->HydrophonesGet.Crc = crcFast((uint8*)DataPacket, 10); // might need to change the number 4 to something else
	    CircularBuffer_Write(TransmitBuffer, &CurrentPacket[0], 14); // need to make sure 14 is the right number to use

	    CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
	    if (U2STAbits.TRMT == 1)
            {
                IFS1bits.U2TXIF = 1;
            }
	}
    }
    /*VARIABLE INITIALIZE SET*/
    else if (PACKETSTATE_PARSEVARIABLESET == CurrentPacketComState)
    {
        CurrentPacketReadCount += CircularBuffer_ReadCopy(ReceiveBuffer,
                &CurrentPacket[CurrentPacketReadCount],
                CurrentPacket[0] - CurrentPacketReadCount);
	if (CurrentPacketReadCount == CurrentPacket[0])
	{
	    if (!PacketComState_CheckCRC())
	    {
		CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
		return;
	    }

            // set the water temperature for calculations
            SpeedofSound = DataPacket->VariableSet.SpeedofSound;
            PingerFreq = DataPacket->VariableSet.PingerFreq;

            PWMConfig();

	    CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
	}
    }
    else
    {
	CurrentPacketComState = PACKETSTATE_READBYTECOUNT;
    }
}

PacketComState PacketComState_ParseFrameId(void)
{
    uint8 frameId = CurrentPacket[1];

    if (frameId < 0x70) // Get
    {
	if (PACKETFRAMEID_HEADING1GET == frameId)
	{
	    return PACKETSTATE_PARSEHEADING1GET;
	}
        else if (PACKETFRAMEID_HEADING2GET == frameId)
	{
	    return PACKETSTATE_PARSEHEADING2GET;
	}
        else if (PACKETFRAMEID_HYDROPHONESGET == frameId)
	{
	    return PACKETSTATE_PARSEHYDROPHONESGET;
	}
	else
	{
	    return PACKETSTATE_READBYTECOUNT;
	}
    }
    else if (frameId > 0x9F) // Set
    {
	if (PACKETFRAMEID_VARIABLESET == frameId)
	{
	    return PACKETSTATE_PARSEVARIABLESET;
	}
	else
	{
	    return PACKETSTATE_READBYTECOUNT;
	}
    }
    else // Alert, not supposed to have
    {
	return PACKETSTATE_READBYTECOUNT;
    }
}

uint8 PacketComState_CheckCRC(void)
{
    uint32 calcedCRC = crcFast(&CurrentPacket[0], CurrentPacket[0] - 4);
    uint8 i = CurrentPacket[0] - 4;
    uint32 actualCRC;

    actualCRC = (uint32)CurrentPacket[i];
    actualCRC += (uint32)CurrentPacket[++i] << 8;
    actualCRC += (uint32)CurrentPacket[++i] << 16;
    actualCRC += (uint32)CurrentPacket[++i] << 24;

    return calcedCRC == actualCRC;
}

void PWMConfig(void)
{
    // Setup Duty Cycle and Period
    // For frequencies between 40kHz and 39kHz
    if(PingerFreq <= 0x28 && PingerFreq >= 0x27)
    {
        FilterDuty = 0x06;
        FilterPeriod = 0x0B;
    }
    // For frequencies between 38kHz and 37kHz
    else if(PingerFreq <= 0x26 && PingerFreq >= 0x25)
    {
        FilterDuty = 0x07;
        FilterPeriod = 0x0C;
    }
    // For frequencies between 36kHz and 35kHz
    else if(PingerFreq <= 0x24 && PingerFreq >= 0x23)
    {
        FilterDuty = 0x07;
        FilterPeriod = 0x0D;
    }
    // For frequencies between 34kHz and 32kHz
    else if(PingerFreq <= 0x22 && PingerFreq >= 0x20)
    {
        FilterDuty = 0x07;
        FilterPeriod = 0x0E;
    }
    // For frequency of 31kHz
    else if(PingerFreq == 0x1F)
    {
        FilterDuty = 0x08;
        FilterPeriod = 0x0F;
    }
    // For frequencies between 30kHz and 29kHz
    else if(PingerFreq <= 0x1E && PingerFreq >= 0x1D)
    {
        FilterDuty = 0x08;
        FilterPeriod = 0x10;
    }
    // For frequency of 28kHz
    else if(PingerFreq == 0x1C)
    {
        FilterDuty = 0x09;
        FilterPeriod = 0x11;
    }
    // For frequencies between 27kHz and 26kHz
    else if(PingerFreq <= 0x1B && PingerFreq >= 0x1A)
    {
        FilterDuty = 0x09;
        FilterPeriod = 0x12;
    }
    // For frequency of 25kHz
    else if(PingerFreq == 0x19)
    {
        FilterDuty = 0x0A;
        FilterPeriod = 0x13;
    }
    else
    {
        FilterDuty = 0x0A;
        FilterPeriod = 0x13;
    }

    // Setup Pwm, and Direction bits to Output
    PIN3_TRIS = OUTPUT;

    // Setup remappable pins to Output Compare
    PIN3_RPO = OC5_O;

    // Set Output Comapre mode to Disabled until init
    OC5CON1bits.OCM = 0b000;

    // Ouput Compare Register setup Duty Cycle
    OC5R = FilterDuty;

    // Allow Output Compare to work when CPU is in Idle Mode
    OC5CON1bits.OCSIDL = 0;

    // Set the PWM Fault pin to reset (Only Used if OCM == 111)
    OC5CON1bits.OCFLT = 0;

    // Set the Clock Source for the Output Compare (Timer 2)
    OC5CON1bits.OCTSEL = 0b000;

    // Set Output Compare Mode to Edge-Aligned PWM mode
    OC5CON1bits.OCM = 0x6;

    // Setup Period Value Register of Timer 2 with PWM frequency
    PR2 = FilterPeriod;

    // Clear Timer 2 interrupt flag
    IFS0bits.T2IF = 0;

    // Start Timer 2 by setting the Timer On bit
    T2CONbits.TON = 0b1;
}