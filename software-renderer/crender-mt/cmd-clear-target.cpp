/*****************************************************************************
*
* cmd-clear-target.cpp
*
* Created By Jari Korkala 1/10/2014
*
*****************************************************************************/

#include <ceng/interfaces/render-target.h>

#include "api-command.h"

#include "crender-context.h"


using namespace Ceng;

Cmd_ClearTarget::Cmd_ClearTarget()
	: renderTarget(nullptr)
{
}

Cmd_ClearTarget::Cmd_ClearTarget(const Ceng::UINT32 id,RenderTarget *renderTarget,const CE_Color &color,
								 std::shared_ptr<RenderState> &renderState)
	: ApiCommand(id),renderTarget(renderTarget),color(color)
{
	this->renderState = renderState;
}

Cmd_ClearTarget::~Cmd_ClearTarget()
{
}

const CRESULT Cmd_ClearTarget::Execute(CR_RenderContext *renderer)
{
	return renderer->Execute_ClearTarget(renderTarget,color,renderState);
}

const CRESULT Cmd_ClearTarget::Prepare(CR_RenderContext *renderer)
{
	return CE_OK;
}