/*****************************************************************************
*
* fragment-format.cpp
*
* By Jari Korkala 2/2012
*
* ---------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to fragment-format.cpp
*
*****************************************************************************/

#include "cr-vertex-shader.h"

#include <ceng/lib/liblog.h>

using namespace Ceng;

/**
 * 
 */
CRESULT CR_FragmentFormat::LinkSemantics(std::vector<CR_vsOutputSemantic> &linkedSemantics,
										 std::vector<CR_vsOutputSemantic> &vshaderOut,
										 std::vector<CR_PixelShaderSemantic> &pshaderIn)
{
	INT32 k,j;

	Log::Print("FragmentFormat.LinkSemantics : method start");

	for(k=0;k<vshaderOut.size();k++)
	{
		if (vshaderOut[k].semantic == Ceng::SHADER_SEMANTIC::POSITION)
		{
			linkedSemantics.push_back(vshaderOut[k]);
			continue;
		}
		
		for(j=0;j<pshaderIn.size();j++)
		{
			if (pshaderIn[j].semantic == Ceng::SHADER_SEMANTIC::POSITION)
			{
				// Pixel shader's POSITION isn't directly related, so skip it
				continue;
			}

			// NOTE: Multiple pixel shader inputs can map into the same vertex shader output

			if (vshaderOut[k].semantic == pshaderIn[j].semantic)
			{
				try
				{
					linkedSemantics.push_back(vshaderOut[k]);
				}
				catch(std::bad_alloc &ba)
				{
					return CE_ERR_OUT_OF_MEMORY;
				}
				break;
			}
		}
	}

	return CE_OK;
}

void CR_FragmentFormat::Reset()
{
	size = 0;

	floatBlocks = 0;
	doubleBlocks = 0;

	floatStart = 0;
	doubleStart = 0;

	positionIsDouble = 0;

	semanticFlags = 0;

	constStart = 0;
	constBlocks = 0;

	variables.clear();
}


/**
 * Constructs intermediate fragment format for clipper and rasterizer.
 * NOTE: Assumes that vertex shader and pixel shader are valid.
 */
CRESULT CR_FragmentFormat::Configure(std::vector<CR_vsOutputSemantic> &vshaderOut,
									 std::vector<CR_PixelShaderSemantic> &pshaderIn)
{
	INT32 k,j;

	//************************
	// Select vertex shader output semantics that are consumed by pixel shader

	std::vector<CR_vsOutputSemantic> linkedSemantic;

	CRESULT cresult;

	cresult = LinkSemantics(linkedSemantic,vshaderOut,pshaderIn);

	if (cresult != CE_OK)
	{
		return cresult;
	}

	Reset();

	UINT32 currentOffset;

	CR_FragmentVariable tempVariable;

	Ceng::SHADER_DATATYPE::value positionFormat;

	//*************************************
	// Find POSITION-semantic and place it first

	for(k=0;k<linkedSemantic.size();k++)
	{
		if (vshaderOut[k].semantic == Ceng::SHADER_SEMANTIC::POSITION)
		{
			positionFormat = vshaderOut[k].dataType;

			tempVariable.semantic = Ceng::SHADER_SEMANTIC::POSITION; 
			tempVariable.format = vshaderOut[k].dataType;
			tempVariable.options = vshaderOut[k].options;

			// Place position as first variable
			tempVariable.offset = 0;

			variables.push_back(tempVariable);
			break;
		}
	}

	if (positionFormat == Ceng::SHADER_DATATYPE::FLOAT4)
	{
		positionIsDouble = 0;

		floatStart = 0;
		currentOffset = 16;

		currentOffset += AssignFloats(currentOffset,linkedSemantic);
		
		floatBlocks = currentOffset >> 4;
		
		doubleStart = currentOffset;

		/*
		currentOffset += AssignDoubles(currentOffset,linkedSemantic);
		doubleBlocks = (currentOffset-doubleStart) >> 4;
		*/
	}
	else if (positionFormat == Ceng::SHADER_DATATYPE::DOUBLE4)
	{
		/*
		positionIsDouble = 1;
		currentOffset = 32;

		currentOffset += AssignDoubles(currentOffset,linkedSemantic);

		doubleBlocks = currentOffset >> 4;

		floatStart = currentOffset;

		currentOffset += AssignFloats(currentOffset,linkedSemantic);

		floatBlocks = (currentOffset-floatStart) >> 4;
		*/
	}
	else
	{
		// Invalid datatype for position
		return CE_ERR_INVALID_PARAM;
	}

	// Write constant block

	/*
	constStart = currentOffset;
	currentOffset = AssignConstants(currentOffset,linkedSemantic);
	constBlocks = (currentOffset-constStart) >> 4;
	*/

	size = currentOffset; 

	Log::Print("\nFragmentFormat.Configure : format dump:\n");

	String temp;

	for(k=0;k<variables.size();k++)
	{
		temp = variables[k].offset;

		temp += ",";

		switch(variables[k].format)
		{
		case SHADER_DATATYPE::FLOAT:
			temp += "float";
			break;
		case SHADER_DATATYPE::FLOAT2:
			temp += "float2";
			break;
		case SHADER_DATATYPE::FLOAT4:
			temp += "float4";
			break;
		default:
			break;
		}

		temp += ",";

		switch(variables[k].semantic)
		{	
		case SHADER_SEMANTIC::POSITION:
			temp += "position";
			break;
		case SHADER_SEMANTIC::NORMAL:
			temp += "normal";
			break;
		case SHADER_SEMANTIC::COLOR_0:
			temp += "color0";
			break;
		case SHADER_SEMANTIC::TEXCOORD_0:
			temp += "texCoord0";
			break;
		case SHADER_SEMANTIC::TEXCOORD_1:
			temp += "texCoord1";
			break;
		case SHADER_SEMANTIC::TEXCOORD_2:
			temp += "texCoord2";
			break;
		default:
			break;		
		}

		Log::Print(temp);
	}

	return CE_OK;
}

Ceng::UINT32 CR_FragmentFormat::AssignFloats(Ceng::UINT32 startOffset,std::vector<CR_vsOutputSemantic> &linkSemantics)
{
	UINT32 currentOffset = startOffset;

	std::vector<UINT32> listVar4;
	std::vector<UINT32> listVar3;
	std::vector<UINT32> listVar2;
	std::vector<UINT32> listVar;

	UINT32 k;

	for(k=0;k<linkSemantics.size();k++)
	{
		// Skip POSITION variable because it's already
		// assigned to offset 0.
		if (linkSemantics[k].semantic == Ceng::SHADER_SEMANTIC::POSITION)
		{
			continue;
		}

		switch(linkSemantics[k].dataType)
		{
		case Ceng::SHADER_DATATYPE::FLOAT:
			listVar.push_back(k);
			break;
		case Ceng::SHADER_DATATYPE::FLOAT2:
			listVar2.push_back(k);
			break;
		case Ceng::SHADER_DATATYPE::FLOAT3:
			listVar3.push_back(k);
			break;
		case Ceng::SHADER_DATATYPE::FLOAT4:
			listVar4.push_back(k);
			break;
		default:
			break;
		}	
	}

	CR_FragmentVariable tempVariable;

	// Assign FLOAT4 blocks

	tempVariable.format = Ceng::SHADER_DATATYPE::FLOAT4;

	for(k=0;k<listVar4.size();k++)
	{
		tempVariable.semantic = linkSemantics[listVar4[k]].semantic;
		tempVariable.options = linkSemantics[listVar4[k]].options;

		tempVariable.offset = currentOffset;
		currentOffset += 16;

		variables.push_back(tempVariable);
	}

	// Assign FLOAT3 blocks

	/*
	tempVariable.format = Ceng::SHADER_DATATYPE::FLOAT3;

	for(k=0;k<listVar3.size();k++)
	{
		tempVariable.semantic = linkSemantics[listVar3[k]].semantic;
		tempVariable.options = linkSemantics[listVar3[k]].options;

		tempVariable.offset = currentOffset;
		currentOffset += 16;

		variables.push_back(tempVariable);
	}
	*/

	//*********************
	// Assign FLOAT variables to excess space of FLOAT3 blocks

	// Calculate number of FLOAT variables that don't fit into FLOAT3 blocks

	/*
	INT32 floatRemainder = listVar.size() - listVar3.size();
	if (floatRemainder < 0)
	{
		floatRemainder = 0;
	}

	if (listVar.size() > 0 && listVar3.size() > 0)
	{
		UINT32 max = listVar.size();
		if (max > listVar3.size())
		{
			max = listVar3.size();
		}

		for(k=0;k<max;k++)
		{
			variables[listVar[k]].offset = variables[listVar3[k]].offset + 12;
		}
	}
	*/

	//*****************************
	// Assign FLOAT2 variables to new blocks
	
	tempVariable.format = Ceng::SHADER_DATATYPE::FLOAT2;
	
	for(k=0;k<listVar2.size();k++)
	{
		tempVariable.semantic = linkSemantics[listVar2[k]].semantic;
		tempVariable.options = linkSemantics[listVar2[k]].options;

		tempVariable.offset = currentOffset;
		currentOffset += 8;

		variables.push_back(tempVariable);
	}
	

	//******************************
	// Append with remaining FLOAT variables

	tempVariable.format = Ceng::SHADER_DATATYPE::FLOAT;

	for(k=0;k<listVar.size();k++)
	{
		tempVariable.semantic = linkSemantics[listVar[k]].semantic;
		tempVariable.options = linkSemantics[listVar[k]].options;
		
		tempVariable.offset = currentOffset;
		currentOffset += 4;

		variables.push_back(tempVariable);
	}

	// Pad to next full block address if necessary

	
	if (currentOffset % 16 > 0)
	{
		currentOffset = 16*(currentOffset >> 4) + 16;
	}
	

	return currentOffset-16;
}

Ceng::UINT32 CR_FragmentFormat::AssignDoubles(Ceng::UINT32 startOffset,std::vector<CR_vsOutputSemantic> &linkedSemantics)
{
	//*********************************************************
	// DOUBLE type variables

	UINT32 currentOffset = startOffset;
	
	return currentOffset;
}

Ceng::UINT32 CR_FragmentFormat::AssignConstants(Ceng::UINT32 startOffset,std::vector<CR_vsOutputSemantic> &linkedSemantics)
{
	//*********************************************************
	// Constant blocks

	return startOffset;
}