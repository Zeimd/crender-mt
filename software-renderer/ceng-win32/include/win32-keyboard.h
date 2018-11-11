/*****************************************************************************
*
* win32-keyboard.h
*
* By Jari Korkala 10/2011
*
* Keyboard device for Windows platform.
*
*****************************************************************************/

#ifndef _CONCEPT2_DINPUT_H
#define _CONCEPT2_DINPUT_H

#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>

#include <ceng/datatypes/keyboard.h>

#include "win32-dll.h"


namespace Ceng
{

	class Keyboard_DirectInput : KeyboardDevice
	{
	protected:
		LPDIRECTINPUT8 core;
		LPDIRECTINPUTDEVICE8 keyboard;
		
		HWND windowHandle;
		HINSTANCE appHandle;
		
		UINT8 keyCodeMap[256];
		UINT8 keyBuffer[256];

		Keyboard_DirectInput();

	public:

		Keyboard_DirectInput(LPDIRECTINPUT8 core,LPDIRECTINPUTDEVICE8 keyboard,
								HWND windowHandle,HINSTANCE appHandle);
		
		virtual ~Keyboard_DirectInput();

		virtual void Release() override;
		
		virtual const Ceng::UINT32 IsPressed(const Ceng::KEYBOARD_KEY::value key);
		virtual const CRESULT Restore();
	};

} // Namespace Ceng

#endif // Include guard