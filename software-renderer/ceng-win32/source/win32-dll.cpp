/*****************************************************************************
*
* win32-dll.cpp
*
* Created By Jari Korkala 11/9/2014
*
*****************************************************************************/

#include "../include/ceng-win32.h"

#include <ceng/platform.h>

#include "../include/win32-dll.h"

using namespace Ceng;

extern "C"
{
	_declspec(dllexport) const Ceng::CRESULT Ceng_LoadLibrary(const Ceng::String &name, Ceng::DynamicLibrary **out_library)
	{
		*out_library = nullptr;

		DLL_Handle *handle = DLL_Handle::GetInstance(name);

		if (handle == nullptr)
		{
			return CE_ERR_FAIL;
		}

		*out_library = handle;

		return CE_OK;		
	}
};

DLL_Handle::DLL_Handle() : module(NULL)
{
}

DLL_Handle::DLL_Handle(const Ceng::String &libraryFile,const ::HMODULE module)
	: libraryFile(libraryFile),module(module)
{
}

DLL_Handle::~DLL_Handle()
{
	if (module != NULL)
	{
		::FreeLibrary(module);
	}
}

void DLL_Handle::Release()
{
	delete this;
}


DLL_Handle* DLL_Handle::GetInstance(const SystemString &libraryFile)
{
#ifdef UNICODE
	return GetInstance(libraryFile.ToWString());
#else
	return GetInstance(libraryFile.ToCString());
#endif
}

DLL_Handle* DLL_Handle::GetInstance(const ::TCHAR *libraryFile)
{
	::HMODULE module = ::LoadLibrary(libraryFile);

	if (module == NULL)
	{
		return nullptr;
	}

	DLL_Handle *temp = new DLL_Handle(libraryFile,module);

	return temp;
}


DynamicCallback DLL_Handle::GetFunction(const Ceng::String &name)
{
	return GetFunction(name.ToCString());
}

DynamicCallback DLL_Handle::GetFunction(const char *name)
{
	return GetProcAddress(module,name);
}

DynamicCallback DLL_Handle::GetFunction(const wchar_t *name)
{
	Ceng::StringUtf8 temp(name);

	return GetProcAddress(module, temp.ToCString());
}

const Ceng::BOOL DLL_Handle::operator == (const DLL_Handle &other)
{
	return module == other.module;
}
	
		
	