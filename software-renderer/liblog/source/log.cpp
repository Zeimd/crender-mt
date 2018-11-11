/*****************************************************************************
*
* log.cpp
*
*****************************************************************************/

#include "../include/liblog.h"

using namespace Ceng;

LogType Log::nullLog = LogType();
LogType* Log::globalLog = &nullLog;

Log::Log()
{
}

Log::~Log()
{
}

CRESULT Log::SetLog(LogType *log)
{
	globalLog = log;
	return CE_OK;
}

void Log::UnsetLog()
{
	globalLog = &nullLog;
}

CRESULT Log::Print(const String &text)
{
	return globalLog->Print(text);
}

