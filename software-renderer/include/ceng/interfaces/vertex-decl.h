/*****************************************************************************
*
* vertex-decl.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_VERTEX_DECL_H
#define _CENG_VERTEX_DECL_H

#include "../datatypes/basic-types.h"

#include "../enums/vtx-datatype.h"
#include "../enums/shader-semantic.h"

namespace Ceng
{
	/**
	* The vertex input format is defined by a list of
	* VertexDeclarationData objects.
	*/
	class VertexDeclData
	{
	public:
		UINT32 streamSource;
		UINT32 offset;
		
		Ceng::VTX_DATATYPE::value format;
		
		Ceng::SHADER_SEMANTIC::value semantic; 
		UINT32 semanticIndex;
	public:
		VertexDeclData() {}
		
		VertexDeclData(UINT32 stream,UINT32 offset,Ceng::VTX_DATATYPE::value format,
							Ceng::SHADER_SEMANTIC::value semantic)
							: streamSource(stream) , offset(offset) ,
							format(format) , semantic(semantic)
		{
		} 
	};

	const VertexDeclData VTX_DECL_END = VertexDeclData(255,255,Ceng::VTX_DATATYPE::FORMAT_END,
																	Ceng::SHADER_SEMANTIC::FORMAT_END);
	
};

#endif