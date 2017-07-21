/**
@author  Mechatronics RoboSub Team 2015
@brief   Handler for main communication states, and data
*/
#ifndef __packetCom__h
#define __packetCom__h

// Includes
#include "types.h"

// Defines

// Typdefs
typedef enum PacketComStateEnum PacketComState;
typedef enum PacketFrameIdEnum PacketComFrameId;

// Structs
enum PacketFrameIdEnum
{
    PACKETFRAMEID_HEADING1GET = 0x60,
    PACKETFRAMEID_HEADING2GET = 0x61,
    PACKETFRAMEID_HYDROPHONESGET = 0x62,
    PACKETFRAMEID_VARIABLESET = 0xF0,
};

enum PacketComStateEnum
{
    PACKETSTATE_READBYTECOUNT,
    PACKETSTATE_READFRAMEID,
    PACKETSTATE_PARSEHEADING1GET,
    PACKETSTATE_PARSEHEADING2GET,
    PACKETSTATE_PARSEHYDROPHONESGET,
    PACKETSTATE_PARSEVARIABLESET
};

// Variables

// Prototypes
/** Runs the state machine for the main communication

@author Petar Tasev
*/
void PacketComState_Run(void);

/** Analyzes the current packet frame id to decide on the next state

@return Returns the next state based on the frame id

@author Petar Tasev
*/
PacketComState PacketComState_ParseFrameId(void);

/** Checks the CRC of the received current packet against a calculated one

@return Returns 1 if the CRC is correct, or 0 if different

@author Petar Tasev
*/
uint8 PacketComState_CheckCRC(void);

#endif
