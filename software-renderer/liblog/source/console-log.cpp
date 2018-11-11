/*****************************************************************************
*
* console-log.cpp
*
* By Jari Korkala 5/2014
*
*****************************************************************************/

#include "../include/liblog.h"

using namespace Ceng;

ConsoleLog::ConsoleLog()
{
}

ConsoleLog::~ConsoleLog()
{
}

CRESULT ConsoleLog::Print(const Ceng::String &text)
{
	return Ceng::CE_OK;
}

LogType& ConsoleLog::operator << (const Ceng::String &source)
{
	std::wcout << source;
	return *this;
}

LogType& ConsoleLog::operator << (const Ceng::String::CONST_REF_TYPE &source)
{
	std::wcout << source;
	return *this;
}

LogType& ConsoleLog::operator << (std::wostream& (*f)(std::wostream&))
{
	f(std::wcout);
	return *this;
}