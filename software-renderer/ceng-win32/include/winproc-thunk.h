/*****************************************************************************
*
* winproc-thunk.h
*
* Created By Jari Korkala 12/9/2014
*
*****************************************************************************/

#ifndef CENG_WINPROC_THUNK_H
#define CENG_WINPROC_THUNK_H

#include <Windows.h>

#include <ceng/datatypes/basic-types.h>


namespace Ceng
{
	class WinAPI_Window;

	class WinProcThunk
	{
	public:

		typedef LRESULT (CALLBACK *Call_Thunk)(::HWND,::UINT,::WPARAM,::LPARAM);

		Call_Thunk thunkCall;

		WinAPI_Window **instancePtrLoc;

		Ceng::UINT32 bufferSize;

		Ceng::UINT8 *codeBuffer;

	protected:

		WinProcThunk(Call_Thunk callback,WinAPI_Window **instancePtrLoc,
						Ceng::UINT8 *codeBuffer,const Ceng::UINT32 bufferSize);

	public:

		void SetWindow(WinAPI_Window *window);

		Call_Thunk GetCallback();

		static WinProcThunk* GetInstance();

		~WinProcThunk();
	};

}

#endif