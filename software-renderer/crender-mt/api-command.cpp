/*****************************************************************************
*
* api-command.cpp
*
* Created By Jari Korkala 30/9/2014
*
*****************************************************************************/

#include "api-command.h"

using namespace Ceng;

ApiCommand::ApiCommand()
{
}

ApiCommand::~ApiCommand()
{
}

ApiCommand::ApiCommand(const Ceng::UINT32 id) : id(id)
{
}

const Ceng::UINT32 ApiCommand::Id() const
{
	return id;
}
