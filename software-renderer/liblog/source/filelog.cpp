/*****************************************************************************
*
* filelog.cpp
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#include "../include/liblog.h"

using namespace Ceng;

FileLog::FileLog()
{
}

FileLog::~FileLog()
{
	CloseFile();
}

CRESULT FileLog::CloseFile()
{
	outputFile.clear();

	if (outputFile.is_open())
	{
		outputFile.flush();
		outputFile.close();

		if (outputFile.bad()) return CE_ERR_FILE_WRITE;
	}
	return CE_OK;
}

CRESULT FileLog::OpenFile(const char *fileName)
{
	outputFile.clear();

	CloseFile();

	outputFile.open(fileName,std::ios_base::out);
	if (outputFile.fail())
	{
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

CRESULT FileLog::Print(const String &text)
{
	if (outputFile.is_open() == false)
	{
		return CE_ERR_FAIL;
	}

	outputFile.clear();

	outputFile << String(text).ToCString() << std::endl;
	if (outputFile.fail())
	{
		return CE_ERR_FILE_WRITE;
	}

	return CE_OK;
}
