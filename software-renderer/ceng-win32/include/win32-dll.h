/*****************************************************************************
*
* win32-dll.h
*
* Created By Jari Korkala 11/9/2014
*
*****************************************************************************/

#ifndef CENG_WIN32_DLL_H
#define CENG_WIN32_DLL_H

#include <Windows.h>

#include <ceng/interfaces/dynamic-library.h>
#include <ceng/datatypes/ce-string.h>

#ifdef UNICODE
	typedef Ceng::StringUtf16 SystemString;
#else
	typedef Ceng::StringUtf8 SystemString;
#endif

namespace Ceng
{
	class DLL_Handle : public DynamicLibrary
	{
	protected:

		SystemString libraryFile;

		::HMODULE module;

		DLL_Handle();
	
		DLL_Handle(const Ceng::String &libraryFile,const ::HMODULE module);
		
	public:

		virtual ~DLL_Handle();

		static DLL_Handle* GetInstance(const SystemString &libraryFile);
		static DLL_Handle* GetInstance(const ::TCHAR *libraryFile);


		virtual DynamicCallback GetFunction(const Ceng::String &name) override;
		virtual DynamicCallback GetFunction(const char *name) override;
		virtual DynamicCallback GetFunction(const wchar_t *name) override;

		const Ceng::BOOL operator == (const DLL_Handle &other);
	
		virtual void Release();
	};
}

#endif