/*****************************************************************************
*
* cr-vertex-format.h
*
* -------------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to cr-vertex-format.h
*
* -------------------------------------------------
*
* Created By Jari Korkala 2/2012
*
* Vertex input format.
*
*****************************************************************************/

#ifndef CRENDER_VERTEX_FORMAT_H
#define CRENDER_VERTEX_FORMAT_H

#include <vector>

#include <ceng/interfaces/vertex-format.h>

// Vertex format interface definitions
#include "crender-base.h"

#include "shader-common.h"

#include "vertex-variable.h"

namespace Ceng
{

	
	//***************************************************************************
	// Vertex format interface

	class CR_VertexFormat : public Ceng::VertexFormat
	{
	public:
		/**
		 * Size of a full vertex (over all streams) in bytes.
		 */
		UINT32 sizeBytes;

		/**
		 * Semantics that are present in input format.
		 */
		UINT32 semanticFlags;
	
		std::vector<CR_VertexVariable> variables;

		/**
		 * For each input stream, number of bytes to
		 * step per vertex.
		 */
		UINT32 streamStrides[CRENDER_MAX_VERTEX_STREAMS];

	public:
		CR_VertexFormat();
		
		virtual ~CR_VertexFormat();
		virtual void Release() override;
	};

} // Namespace end

#endif // Include guard