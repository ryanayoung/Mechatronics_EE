/**
@author  Mechatronics RoboSub Team 2015
@brief   Functions to manage transcieved data
*/
#ifndef __circularBuffer__h
#define __circularBuffer__h

// Includes
#include "types.h"

// Defines
#define CIRC_BUFFER_SIZE 240
#define CircularBuffer_GetCount(BUF) (BUF->WriteCount - BUF->ReadCount)

// Typedefs
typedef struct CircularBufferStruct CircularBuffer;

// Structs
/** Circular buffer with ability to read and write bytes

@author Petar Tasev
*/
struct CircularBufferStruct
{
    uint8 Buffer[CIRC_BUFFER_SIZE];
    uint16 Start;
    uint16 End;
};

// Variables

// Prototypes
/** Initializes and allocates memory for a new CircularCommunicationBuffer

@author Petar Tasev
*/
CircularBuffer* CircularBuffer_Create(void);

/** Writes bytes of data into the buffer

@param buffer The CircularBuffer to which data will be written
@param data The array pointer to the bytes that will be written
@param length The length of the data array being passed in

@author Petar Tasev
*/
void CircularBuffer_Write(CircularBuffer* buffer, uint8 data[], uint16 length);

/** Returns the next entry ready to be processed.

@param buffer The CircularBuffer from which data will be read
@param data The pointer to the destination of the data
@return Returns 1 if successful, and 0 if failed

@author Petar Tasev
*/
uint8 CircularBuffer_Read(CircularBuffer* buffer, uint8* data);

/** Returns the next entry ready to be processed.

@param buffer The CircularBuffer from which data will be read
@param data The array pointer to the destination of the copied entries
@param length The amount of data to be copied
@return Returns 1 if successful, and 0 if failed

@author Petar Tasev
*/
uint8 CircularBuffer_ReadCopy(CircularBuffer* buffer, uint8 data[], uint16 length);

#endif
