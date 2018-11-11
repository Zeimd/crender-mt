/*****************************************************************************
*
* eng-actor.cpp
*
* By Jari Korkala 10/2011
*
*****************************************************************************/

#include "../include/engine-v2.h"

Actor::Actor()
{
	mesh = NULL;
}

Actor::~Actor()
{
}

Ceng::CRESULT Actor::SetMesh(Mesh *meshPtr)
{
	if (mesh == NULL)
	{
		return Ceng::CE_ERR_FAIL;
	}

	mesh = (Mesh*)meshPtr;

	return Ceng::CE_OK;
}

