/*****************************************************************************
*
* vertex-variable.h
*
* By Jari Korkala 8/2014
*
* Class CR_VertexVariable moved here.
*
*****************************************************************************/

#ifndef CRENDER_VERTEX_VARIABLE_H
#define CRENDER_VERTEX_VARIABLE_H

#include <ceng/enums/shader-semantic.h>
#include <ceng/enums/vtx-datatype.h>

#include "crender-base.h"

namespace Ceng
{
	//*********************************************************************
	// Vertex input format data structures

	/**
	 * Contains per-variable data about the vertex format.
	 * Used to gather input data from multiple streams.
	 */
	class CR_VertexVariable
	{
	public:
		/**
		 * One of the CR_VTX_VARIABLE_FLAG values.
		 */
		Ceng::SHADER_SEMANTIC::value semantic;

		Ceng::VTX_DATATYPE::value dataType;

		/**
		 * Index of the input stream this variable is read from.
		 */
		Ceng::UINT32 inputStream;

		/**
		 * Offset from stream base address. 
		 * NOTE : Stream step size (stride)  is stored in 
		 *        CR_VertexFormatHeader.
		 */
		Ceng::UINT32 inputOffset; 

		/**
		 * Offset in assembled input vertex struct.
		 */
		Ceng::UINT32 outputOffset; 

		/**
		 * Set to 1 if the variable is aligned to 16
		 * byte boundary in its stream, and stream
		 * stride is divisible by 16.
		 */
		Ceng::UINT32 aligned;

		/**
		 * Pad to a power of 2.
		 */
		Ceng::UINT32 padding[2];

	public:
		CR_VertexVariable()
		{
			semantic = SHADER_SEMANTIC::FORMAT_END;
			dataType = VTX_DATATYPE::UNKNOWN;
			inputStream = 0;
			inputOffset = 0;
			outputOffset = 0;
			aligned = 0;
		}
	};

}

#endif