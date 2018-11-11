/*****************************************************************************
*
* winproc-thunk.cpp
*
* Created By Jari Korkala 12/9/2014
*
*****************************************************************************/

#include "../include/winproc-thunk.h"

#include "../include/win32-window.h"

using namespace Ceng;

typedef LRESULT (__thiscall WinAPI_Window::*Call_WindowProc)(::HWND,::UINT,::WPARAM,::LPARAM);

const Call_WindowProc call_WindowProc = &WinAPI_Window::WindowProc;

typedef LRESULT (*WrapperCall)(WinAPI_Window*,::HWND,::UINT,::WPARAM,::LPARAM);

/**
 * Let the compiler handle forwarding to correct window object's
 * member function.
 */
LRESULT Wrapper(WinAPI_Window *instance,::HWND hwnd,::UINT msg,::WPARAM wParam,::LPARAM lParam)
{
	return instance->WindowProc(hwnd,msg,wParam,lParam);
}

WinProcThunk* WinProcThunk::GetInstance()
{
	Ceng::UINT8 *codeBuffer = (Ceng::UINT8*)::VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);

	if (codeBuffer == nullptr)
	{
		return nullptr;
	}

	// mov eax,esp
	codeBuffer[0] = 0x8b;
	codeBuffer[1] = (3 << 6) + (0 << 3) + 4;

	// push [eax+16] = LPARAM
	codeBuffer[2] = 0xff;
	codeBuffer[3] = (1 << 6) + (6 << 3) + 0;
	codeBuffer[4] = 16;

	// push [eax+12] = WPARAM
	codeBuffer[5] = 0xff;
	codeBuffer[6] = (1 << 6) + (6 << 3) + 0;
	codeBuffer[7] = 12;

	// push [eax+8] = MSG
	codeBuffer[8] = 0xff;
	codeBuffer[9] = (1 << 6) + (6 << 3) + 0;
	codeBuffer[10] = 8;

	// push [eax+4] = HWND
	codeBuffer[11] = 0xff;
	codeBuffer[12] = (1 << 6) + (6 << 3) + 0;
	codeBuffer[13] = 4;
	

	// push [imm32]
	codeBuffer[14] = 0x68;
	codeBuffer[15] = 0;
	codeBuffer[16] = 0;
	codeBuffer[17] = 0;
	codeBuffer[18] = 0;

	WinAPI_Window **instancePtrLoc = (WinAPI_Window**)&codeBuffer[15];

	// mov eax,[call_Wrapper]
	codeBuffer[19] = 0xb8+0;
	codeBuffer[20] = 0;
	codeBuffer[21] = 0;
	codeBuffer[22] = 0;
	codeBuffer[23] = 0;

	WrapperCall *ptr = (WrapperCall*)&codeBuffer[20];

	*ptr = &Wrapper;
	
	// call [eax]
	codeBuffer[24] = 0xff;
	codeBuffer[25] = (3 << 6) + (2 << 3);

	// add esp,20
	codeBuffer[26] = 0x83;
	codeBuffer[27] = (3 << 6) + 4;
	codeBuffer[28] = 20;

	// ret 16
	codeBuffer[29] = 0xc2;
	codeBuffer[30] = 0;
	codeBuffer[31] = 0;

	// Contents of the codeBuffer are equivalent to the following
	// assembly code:
	
	/*
	__asm
	{
		mov eax,esp;

		push [eax+16]; // LPARAM
		push [eax+12]; // WPARAM
		push [eax+8]; // UINT msg
		push [eax+4]; // HWND
		push instance

		mov eax,call_WindowProc;

		call [eax];

		add esp,20
		
		return 16
	}
	*/

	Call_Thunk thunkPtr = (Call_Thunk)&codeBuffer[0];

	WinProcThunk *temp = new WinProcThunk(thunkPtr,instancePtrLoc,codeBuffer,4096);

	return temp;
}

WinProcThunk::WinProcThunk(Call_Thunk callback,WinAPI_Window **instancePtrLoc,
						   Ceng::UINT8 *codeBuffer,
						   const Ceng::UINT32 bufferSize)
	: thunkCall(callback),instancePtrLoc(instancePtrLoc),codeBuffer(codeBuffer),bufferSize(bufferSize)
{
	this->bufferSize = bufferSize;
}

void WinProcThunk::SetWindow(WinAPI_Window *window)
{
	*instancePtrLoc = window;
}


WinProcThunk::Call_Thunk WinProcThunk::GetCallback()
{
	return thunkCall;
}

WinProcThunk::~WinProcThunk()
{
	::VirtualFree(codeBuffer,bufferSize,MEM_RELEASE);
}

