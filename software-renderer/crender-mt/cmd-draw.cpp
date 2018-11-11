/*****************************************************************************
*
* cmd-draw.cpp
*
* Created By Jari Korkala 1/10/2014
*
*****************************************************************************/

#include "api-command.h"

#include "crender-context.h"

using namespace Ceng;

Cmd_DrawPrimitive::Cmd_DrawPrimitive()
	: ApiCommand(0),type(PRIMITIVE_TYPE::FORCE_32B),firstVertex(0),amount(0)
{
}

Cmd_DrawPrimitive::Cmd_DrawPrimitive(const Ceng::UINT32 id,
									 const Ceng::PRIMITIVE_TYPE::value type,
									 const Ceng::UINT32 firstVertex,
									 const Ceng::UINT32 amount,
									 std::shared_ptr<RenderState> &renderState)
									 : ApiCommand(id),type(type),firstVertex(firstVertex),amount(amount)
{
	this->renderState = renderState;
}

Cmd_DrawPrimitive::~Cmd_DrawPrimitive()
{
}

const CRESULT Cmd_DrawPrimitive::Execute(CR_RenderContext *renderer)
{
	return renderer->Execute_DrawPrimitive(id,type,firstVertex,amount,renderState);
}

const CRESULT Cmd_DrawPrimitive::Prepare(CR_RenderContext *renderer)
{
	return CE_OK;
}
