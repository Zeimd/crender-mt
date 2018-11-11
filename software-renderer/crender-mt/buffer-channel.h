/*****************************************************************************
*
* buffer-channel.h
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_BUFFER_CHANNEL_H
#define _CONCEPT2_BUFFER_CHANNEL_H

#include <vector>

#include <ceng/datatypes/abstract-data.h>

#include "crender-base.h"

namespace Ceng
{
	// Buffer channel semantics
	//
	// Can be combined. Grayscale = BLUE | GREEN | RED on the same channel

	namespace CHANNEL_SEMANTIC
	{
		enum value
		{
			UNDEFINED = 1 , // User-defined data
			
			// Contains packed color of buffer format
			FORMAT_COLOR = 2 ,
			
			// Individual color channels
			BLUE = 1 << 2 ,
			GREEN = 1 << 3 ,
			RED = 1 << 4 ,
			ALPHA = 1 << 5 ,
			
			// Contains packed depth-stencil of buffer format
			FORMAT_DEPTH_STENCIL = 1 << 6 ,

			// Individual depth and stencil channels
			DEPTH = 1 << 7 ,
			STENCIL = 1 << 8 ,	

			MASK_COLOR = BLUE | GREEN | RED | ALPHA ,
			MASK_DEPTH_STENCIL = DEPTH | STENCIL ,

			FORCE32B = UINT32(1) << 30 ,
		};
	}

	namespace CHANNEL_DATA
	{
		// NOTE: Only power of two values for fast testing
		enum value
		{
			INT = 1 ,
			UINT = 2 ,
			
			BOOL = 4 ,
			
			// Floating point data types force channel width in bits
			FLOAT32 = 8 , 
			FLOAT64 = 16 ,
			
			FLOAT16 = 32 ,

			// Channel contents is defined by buffer format
			FORMAT_DEFINED = 64 ,

			UNKNOWN = 128 ,

			MASK_FLOAT = FLOAT16 | FLOAT32 | FLOAT64 ,

			MASK_INT = INT | UINT | BOOL ,
			
			FORCE32B = UINT32(1) << 30 ,
		};
	}

	// NOTE: Only power of twos
	namespace CHANNEL_OPTION
	{
		enum value
		{
			// If converted to float, rescale unsigned int to range [0,1]
			// and signed int to [-1,1]
			
			//
			// NOTE: floating point data is scaled with destination
			//       format's maximum value (signed/unsigned) before
			//       conversion
			
			NORMALIZED = 1 , 
			
			// If a value is written to the channel, saturate to given
			// min/max values
			
			MIN_SATURATED = 2 ,
			MAX_SATURATED = 4 ,
			
			FORCE32B = UINT32(1) << 30 ,
		};
	}

	namespace VECTOR_TYPE
	{
		enum value
		{
			INT8 = 1 ,
			INT16 = 2 ,
			INT32 = 3 ,

			FLOAT16 = 4 ,
			FLOAT32 = 5 ,
			FLOAT64 = 6 ,

			FORCE32B = UINT32(1) << 30 ,
		};
	}

	struct CR_ChannelData
	{
		Ceng::CHANNEL_SEMANTIC::value semantic;
		Ceng::VECTOR_TYPE::value dataType;
		
		Ceng::BOOL mixedData;

		INT32 linearBytes; // Bytes allocated for a linear unit of this channel

		INT32 bytesPerPixel;
		INT32 bufferOffset;
		
		INT32 tileXstep; // Steps one tile in x-direction
		INT32 tileYstep; // Steps one tile in y-direction		

		// Steps one linear unit in y-direction within tile
		INT32 unitYstep; 

		// Steps one linear unit in x-direction within tile
		INT32 unitXstep;

		// Offset to reach each pixel in a quad from the top-left
		// pixel.
		//
		// NOTE: quadSteps[0] = 0 in order to eliminate branches
		INT32 quadSteps[4];
	};

	class CR_BufferChannel
	{
	public:
		// Saturation values if used
		Abstract<8> minimum;
		Abstract<8> maximum;

		UINT32 semantics; // A combination of CHANNEL_SEMANTIC

		CHANNEL_DATA::value dataType;

		// A combination of CHANNEL_OPTIONS
		UINT32 options;		

		// Width of the channel in bits. Fixed for FLOAT,DOUBLE
		UINT32 bits; 

		// Size of the variable used to access the channel data in bytes
		UINT32 accessVarBytes; 

		// Bit position within access variable
		UINT32 bitPos;

		// Byte offset of access variable within stream
		POINTER streamOffset;

		// Offset of stream within buffer
		POINTER bufferOffset;

	public:
		CR_BufferChannel()
		{
		}
	};

	class CR_BufferStream
	{
	public:
		std::vector<CR_BufferChannel> channels;

		Ceng::CHANNEL_DATA::value dataType; // Access data type for the channel
		UINT32 structSize; // Size of channel struct in bytes

		POINTER bufferOffset; // Offset of the stream within buffer

	public:
		CR_BufferStream();	
		~CR_BufferStream();

		void Reset();

		UINT32 FreeBits();

		CRESULT SetDataType(const Ceng::CHANNEL_DATA::value type);

		CRESULT SetBufferOffset(const POINTER offset);

		CRESULT AddChannel(CR_BufferChannel &newChannel);

	protected:
		
		void Configure();
	};

	inline UINT32 CeilPow2(const UINT32 value)
	{
		UINT32 k;
		UINT32 ceilPow = 1;	
		
		for(k=0;k<32;k++)
		{
			if (ceilPow > value)
			{
				break;
			}
			ceilPow = ceilPow << 1;
		}
		return ceilPow;
	}

} // namespace Ceng

#endif // Include guard