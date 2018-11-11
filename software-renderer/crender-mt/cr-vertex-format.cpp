/*****************************************************************************
*
* cr-vertex-format.cpp
*
* By Jari Korkala 9/2011
*
* Implements CR_VertexFormat
*
* ------------------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to cr-vertex-format 
*
*****************************************************************************/


#include "cr-vertex-format.h"

using namespace Ceng;

CR_VertexFormat::CR_VertexFormat()
{
}

CR_VertexFormat::~CR_VertexFormat()
{
	//variables.clear();
	//variables = std::vector<CR_VertexVariable>();
}

void CR_VertexFormat::Release()
{
	delete this;
}