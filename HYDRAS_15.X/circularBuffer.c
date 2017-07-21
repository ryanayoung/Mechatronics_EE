// Includes
#include "circularBuffer.h"
#include <stdlib.h>

CircularBuffer* CircularBuffer_Create(void)
{
  CircularBuffer* buffer = (CircularBuffer*)malloc(sizeof(CircularBuffer));

  if (NULL != buffer)
  {
    buffer->Start = 0;
    buffer->End = 0;
  }

  return buffer;
}

void CircularBuffer_Write(CircularBuffer* buffer, uint8 data[], uint16 length)
{
	uint16 i;
	for (i = 0; i < length; ++i)
	{
	    buffer->Buffer[buffer->End] = data[i];

	    ++buffer->End;
	    if (CIRC_BUFFER_SIZE == buffer->End)
            {
		buffer->End = 0;
            }
	}
}

uint8 CircularBuffer_Read(CircularBuffer* buffer, uint8* data)
{
    return CircularBuffer_ReadCopy(buffer, data, 1);
//    if ((buffer->WriteCount - buffer->ReadCount) == 0)
//    {
//	return 0;
//    }
//    else
//    {
//	*data = buffer->Buffer[buffer->Start];
//
//	++buffer->Start;
//	if (CIRC_BUFFER_SIZE == buffer->Start)
//	{
//	    buffer->Start = 0;
//	}
//
//	return 1;
//    }
}

uint8 CircularBuffer_ReadCopy(CircularBuffer* buffer, uint8 data[], uint16 length)
{
    uint16 i = 0;
    while (buffer->Start != buffer->End && i < length)
    {
        data[i] = buffer->Buffer[buffer->Start];

        ++buffer->Start;
        if (CIRC_BUFFER_SIZE == buffer->Start)
        {
            buffer->Start = 0;
        }
        ++i;
    }

    return i;
//    if ((buffer->WriteCount - buffer->ReadCount) >= length)
//    {
//	for (i = 0; i < length; ++i)
//	{
//	    data[i] = buffer->Buffer[buffer->Start];
//	    ++buffer->ReadCount;
//
//	    ++buffer->Start;
//	    if (CIRC_BUFFER_SIZE == buffer->Start)
//	    {
//		buffer->Start = 0;
//	    }
//	}
//
//	return 1;
//    }
//    else
//    {
//	return 0;
//    }
}


