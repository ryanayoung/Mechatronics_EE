/**
@author  Mechatronics RoboSub Team 2015
@brief   Structures for all of the different data types
 *
 * The __pack specifier may be used to indicate that structures should not use memory
gaps to align structure members, or that individual structure members should not be
aligned.
*/
#ifndef __packets__h
#define __packets__h

// Includes
#include "types.h"

// Defines
#define MAX_PACKET_BYTECOUNT 16
#define MIN_PACKET_BYTECOUNT 6

// Typedefs
typedef union PacketUnion Packet;

// Structures
struct PacketHeaderStruct
{
    uint8 ByteCount;
    uint8 FrameId;
};

struct PacketPingerHeading1GetStruct
{
    struct PacketHeaderStruct Header;
    uint16 Heading1;
    uint16 AOI1;
    uint8 Confidence1;
    __pack uint32 Crc;
};

struct PacketPingerHeading2GetStruct
{
    struct PacketHeaderStruct Header;
    uint16 Heading2;
    uint16 AOI2;
    uint8 Confidence2;
    __pack uint32 Crc;
};

struct PacketHydrophonesGetStruct
{
    struct PacketHeaderStruct Header;
    uint16 H1;
    uint16 H2;
    uint16 H3;
    uint16 H4;
    uint32 Crc;
};

struct PacketVariableSetStruct
{
    struct PacketHeaderStruct Header;
    uint16 SpeedofSound;
    uint8 PingerFreq;
    __pack uint32 Crc;
};

union PacketUnion
{
    struct PacketHeaderStruct Header;
    struct PacketPingerHeading1GetStruct Heading1Get;
    struct PacketPingerHeading2GetStruct Heading2Get;
    struct PacketHydrophonesGetStruct HydrophonesGet;
    struct PacketVariableSetStruct VariableSet;
};

// Variables

// Prototypes

#endif
