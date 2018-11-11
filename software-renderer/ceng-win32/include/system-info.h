/*****************************************************************************
*
* system-info.h
*
* By Jari Korkala 18/6/2011
*
* NOTE: Do not include this file directly. Include "ce-platform.h" instead.
*
*****************************************************************************/

#ifndef _CONCEPT2_PLATFORM_WIN32_H
#define _CONCEPT2_PLATFORM_WIN32_H

//****************************************************************************
// OS: Windows , x86

#ifdef _MSC_VER
#ifdef _WIN32

#define _CE_PLATFORM_WIN32

#include <windows.h>

#include <ceng/datatypes/return-val.h>

namespace Ceng
{
	class Win32_SystemInfo
	{
	public:

		SYSTEM_INFO systemInfo;
	
		// Multi-core cpu data
		//std::vector<ULONG_PTR> coreID; // Affinity mask for each core
		
		// Process / thread data
		::HANDLE processHandle;
	
		::BOOL isWOW64;

	public:

		Win32_SystemInfo();
		~Win32_SystemInfo();	

		const CRESULT Configure();
	};

} // Namespace Ceng




#endif // _WIN32
#endif // _MSC_VER

#endif // Include guard