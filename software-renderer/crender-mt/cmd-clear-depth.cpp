/*****************************************************************************
*
* cmd-clear-depth.cpp
*
* Created By Jari Korkala 1/10/2014
*
*****************************************************************************/

#include "api-command.h"

#include "crender-context.h"

using namespace Ceng;

Cmd_ClearDepth::Cmd_ClearDepth()
	: depth(1.0f)
{
}

Cmd_ClearDepth::Cmd_ClearDepth(const Ceng::UINT32 id,const Ceng::FLOAT32 depth,
							   std::shared_ptr<RenderState> &renderState)
	: ApiCommand(id),depth(depth)
{
	this->renderState = renderState;
}

Cmd_ClearDepth::~Cmd_ClearDepth()
{
}

const CRESULT Cmd_ClearDepth::Execute(CR_RenderContext *renderer)
{
	return renderer->Execute_ClearDepth(depth,renderState);
}

const CRESULT Cmd_ClearDepth::Prepare(CR_RenderContext *renderer)
{
	return CE_OK;
}

//************************************************************************
// class ClearDepthStencil

Cmd_ClearDepthStencil::Cmd_ClearDepthStencil()
	: depth(1.0f),stencil(0)
{
}

Cmd_ClearDepthStencil::Cmd_ClearDepthStencil(const Ceng::UINT32 id, Ceng::FLOAT32 depth,
											 const Ceng::UINT32 stencil,
											 std::shared_ptr<RenderState> &renderState)
	: ApiCommand(id),depth(depth),stencil(stencil)
{
	this->renderState = renderState;
}

Cmd_ClearDepthStencil::~Cmd_ClearDepthStencil()
{
}

const CRESULT Cmd_ClearDepthStencil::Execute(CR_RenderContext *renderer)
{
	return renderer->Execute_ClearDepthStencil(depth,stencil,renderState);
}

const CRESULT Cmd_ClearDepthStencil::Prepare(CR_RenderContext *renderer)
{
	return CE_OK;
}