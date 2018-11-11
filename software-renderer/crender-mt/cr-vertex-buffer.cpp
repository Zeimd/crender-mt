/*****************************************************************************
*
* cr-vertex-format.cpp
*
*----------------------------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to cr-vertex-format.cpp
*
* -------------------------------------------------------
*
* Created By Jari Korkala 9/2011
*
* Vertex buffer for the software Concept Renderer.
*
* --------------------------------------------------------
*
* 
*
*****************************************************************************/

#include "cr-vertex-buffer.h"

using namespace Ceng;

CR_VertexBuffer::CR_VertexBuffer()
{
	vertexBuffer = nullptr;
	elementSizeBytes = 0;
	elementCount = 0;
}

CR_VertexBuffer::CR_VertexBuffer(Ceng::UINT8 *ptr,const Ceng::UINT32 newSize,const Ceng::UINT32 elements)
{
	vertexBuffer = ptr;
	elementSizeBytes = newSize;
	elementCount = elements;
}

CR_VertexBuffer::~CR_VertexBuffer()
{
	if (vertexBuffer != nullptr)
	{
		AlignedFree(vertexBuffer);
		vertexBuffer = nullptr;
	}
	elementSizeBytes = 0;
	elementCount = 0;
}

void CR_VertexBuffer::Release()
{
	delete this;
}

const BufferType::value CR_VertexBuffer::GetResourceType()
{
	return BufferType::buffer;
}

const CRESULT CR_VertexBuffer::ElementSize(Ceng::UINT32 *size)
{
	if (vertexBuffer == nullptr)
	{
		return CE_ERR_FAIL;
	}
	*size = elementSizeBytes;
	return CE_OK;
}

const CRESULT CR_VertexBuffer::ElementCount(Ceng::UINT32 *elements)
{
	if (vertexBuffer == nullptr)
	{
		return CE_ERR_FAIL;
	}
	*elements = elementCount;
	return CE_OK;
}

const CRESULT CR_VertexBuffer::BufferPtr(Ceng::UINT8 **bufferPtr)
{
	if (vertexBuffer == nullptr)
	{
		return CE_ERR_FAIL;
	}

	*bufferPtr = vertexBuffer;
	return CE_OK;
}