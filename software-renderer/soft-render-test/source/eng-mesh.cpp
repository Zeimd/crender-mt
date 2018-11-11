/*****************************************************************************
*
* eng-mesh.cpp
*
* By Jari Korkala 10/2011
*
*****************************************************************************/

#include "../include/engine-v2.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	vertices.clear();
	vertices = std::vector<ProgVertexData>();

	surfaces.clear();
	surfaces = std::vector<SurfaceData>();
}