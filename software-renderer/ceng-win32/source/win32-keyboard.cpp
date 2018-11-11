/*****************************************************************************
*
* win32-keyboard.cpp
*
* By Jari Korkala 10/2011
*
* Keyboard device methods for Windows.
*
*****************************************************************************/

#include <exception>

#include "../include/ceng-win32.h"

#include "../include/win32-keyboard.h"
#include "../include/win32-window.h"

using namespace Ceng;

extern "C" _declspec(dllexport) const Ceng::CRESULT Ceng_GetKeyboard(Ceng::PlatformWindow *focusWindow,
												Ceng::KeyboardDevice **out_device)
{
	if (focusWindow == nullptr)
	{
		return CE_ERR_INVALID_PARAM;
	}
	
	LPDIRECTINPUT8 core;

	HRESULT hresult;

	hresult = DirectInput8Create(::GetModuleHandle(NULL),DIRECTINPUT_VERSION,
									IID_IDirectInput8,
									(LPVOID*)&core,nullptr);

	if (hresult != DI_OK)
	{
		return CE_ERR_FAIL;
	}

	LPDIRECTINPUTDEVICE8 keyboard;

	// Create keyboard device
	hresult = core->CreateDevice(GUID_SysKeyboard,&keyboard,nullptr);

	if (hresult != DI_OK)
	{
		core->Release();
		return CE_ERR_FAIL;
	}

	// Set data format
	hresult = keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (hresult != DI_OK)
	{
		keyboard->Release();
		core->Release();

		return CE_ERR_FAIL;
	}

	WinAPI_Window *windowPtr = (WinAPI_Window*)focusWindow;
	
	// Set cooperative level
	hresult = keyboard->SetCooperativeLevel(windowPtr->GetWindowHandle(),
											DISCL_NONEXCLUSIVE|
											DISCL_FOREGROUND);
	if (hresult != DI_OK)
	{
		keyboard->Release();
		core->Release();		

		return CE_ERR_FAIL;
	}

	Keyboard_DirectInput *temp;

	temp = new Keyboard_DirectInput(core,keyboard,windowPtr->GetWindowHandle(),
										::GetModuleHandle(NULL));

	if (temp == nullptr)
	{
		keyboard->Release();
		core->Release();

		return CE_ERR_OUT_OF_MEMORY;
	}

	*out_device = (Ceng::KeyboardDevice*)temp;

	return CE_OK;
}

Keyboard_DirectInput::Keyboard_DirectInput(LPDIRECTINPUT8 core,LPDIRECTINPUTDEVICE8 keyboard,
										   HWND windowHandle,HINSTANCE appHandle)
	: core(core),keyboard(keyboard),windowHandle(windowHandle),appHandle(appHandle)
{
	memset(keyBuffer,0,256);

	keyCodeMap[Ceng::KEYBOARD_KEY::A] = DIK_A;
	keyCodeMap[Ceng::KEYBOARD_KEY::B] = DIK_B;
	keyCodeMap[Ceng::KEYBOARD_KEY::C] = DIK_C;
	keyCodeMap[Ceng::KEYBOARD_KEY::D] = DIK_D;
	keyCodeMap[Ceng::KEYBOARD_KEY::E] = DIK_E;
	keyCodeMap[Ceng::KEYBOARD_KEY::F] = DIK_F;
	keyCodeMap[Ceng::KEYBOARD_KEY::G] = DIK_G;
	keyCodeMap[Ceng::KEYBOARD_KEY::H] = DIK_H;
	keyCodeMap[Ceng::KEYBOARD_KEY::I] = DIK_I;
	keyCodeMap[Ceng::KEYBOARD_KEY::J] = DIK_J;
	keyCodeMap[Ceng::KEYBOARD_KEY::K] = DIK_K;
	keyCodeMap[Ceng::KEYBOARD_KEY::L] = DIK_L;
	keyCodeMap[Ceng::KEYBOARD_KEY::M] = DIK_M;
	keyCodeMap[Ceng::KEYBOARD_KEY::N] = DIK_N;
	keyCodeMap[Ceng::KEYBOARD_KEY::O] = DIK_O;
	keyCodeMap[Ceng::KEYBOARD_KEY::P] = DIK_P;
	keyCodeMap[Ceng::KEYBOARD_KEY::Q] = DIK_Q;
	keyCodeMap[Ceng::KEYBOARD_KEY::R] = DIK_R;
	keyCodeMap[Ceng::KEYBOARD_KEY::S] = DIK_S;
	keyCodeMap[Ceng::KEYBOARD_KEY::T] = DIK_T;
	keyCodeMap[Ceng::KEYBOARD_KEY::U] = DIK_U;
	keyCodeMap[Ceng::KEYBOARD_KEY::V] = DIK_V;
	keyCodeMap[Ceng::KEYBOARD_KEY::W] = DIK_W;
	keyCodeMap[Ceng::KEYBOARD_KEY::X] = DIK_X;
	keyCodeMap[Ceng::KEYBOARD_KEY::Y] = DIK_Y;
	keyCodeMap[Ceng::KEYBOARD_KEY::Z] = DIK_Z;

	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_1] = DIK_1;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_2] = DIK_2;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_3] = DIK_3;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_4] = DIK_4;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_5] = DIK_5;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_6] = DIK_6;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_7] = DIK_7;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_8] = DIK_8;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_9] = DIK_9;
	keyCodeMap[Ceng::KEYBOARD_KEY::KEY_0] = DIK_0;

	keyCodeMap[Ceng::KEYBOARD_KEY::MINUS] = DIK_MINUS;
	keyCodeMap[Ceng::KEYBOARD_KEY::EQUALS] = DIK_EQUALS;
	
	keyCodeMap[Ceng::KEYBOARD_KEY::BACKSLASH] = DIK_BACKSLASH;

	keyCodeMap[Ceng::KEYBOARD_KEY::LEFT_BRACKET] = DIK_LBRACKET;
	keyCodeMap[Ceng::KEYBOARD_KEY::RIGHT_BRACKET] = DIK_RBRACKET;
	
	keyCodeMap[Ceng::KEYBOARD_KEY::COMMA] = DIK_COMMA;
	keyCodeMap[Ceng::KEYBOARD_KEY::PERIOD] = DIK_PERIOD;
	keyCodeMap[Ceng::KEYBOARD_KEY::SLASH] = DIK_SLASH;

	keyCodeMap[Ceng::KEYBOARD_KEY::SEMICOLON] = DIK_SEMICOLON;
	keyCodeMap[Ceng::KEYBOARD_KEY::APOSTROPHE] = DIK_APOSTROPHE;
	keyCodeMap[Ceng::KEYBOARD_KEY::GRAVE] = DIK_GRAVE;

	keyCodeMap[Ceng::KEYBOARD_KEY::SPACE] = DIK_SPACE;
	keyCodeMap[Ceng::KEYBOARD_KEY::ENTER] = DIK_RETURN;
	keyCodeMap[Ceng::KEYBOARD_KEY::BACKSPACE] = DIK_BACK;

	keyCodeMap[Ceng::KEYBOARD_KEY::TAB] = DIK_TAB;
	keyCodeMap[Ceng::KEYBOARD_KEY::CAPS] = DIK_CAPSLOCK;
	keyCodeMap[Ceng::KEYBOARD_KEY::LEFT_SHIFT] = DIK_LSHIFT;
	keyCodeMap[Ceng::KEYBOARD_KEY::RIGHT_SHIFT] = DIK_RSHIFT;

	keyCodeMap[Ceng::KEYBOARD_KEY::ESC] = DIK_ESCAPE;
	keyCodeMap[Ceng::KEYBOARD_KEY::F1] = DIK_F1;
	keyCodeMap[Ceng::KEYBOARD_KEY::F2] = DIK_F2;
	keyCodeMap[Ceng::KEYBOARD_KEY::F3] = DIK_F3;
	keyCodeMap[Ceng::KEYBOARD_KEY::F4] = DIK_F4;
	keyCodeMap[Ceng::KEYBOARD_KEY::F5] = DIK_F5;
	keyCodeMap[Ceng::KEYBOARD_KEY::F6] = DIK_F6;
	keyCodeMap[Ceng::KEYBOARD_KEY::F7] = DIK_F7;
	keyCodeMap[Ceng::KEYBOARD_KEY::F8] = DIK_F8;
	keyCodeMap[Ceng::KEYBOARD_KEY::F9] = DIK_F9;
	keyCodeMap[Ceng::KEYBOARD_KEY::F10] = DIK_F10;
	keyCodeMap[Ceng::KEYBOARD_KEY::F11] = DIK_F11;
	keyCodeMap[Ceng::KEYBOARD_KEY::F12] = DIK_F12;

	keyCodeMap[Ceng::KEYBOARD_KEY::NUMLOCK] = DIK_NUMLOCK;
	keyCodeMap[Ceng::KEYBOARD_KEY::SCROLL_LOCK] = DIK_SCROLL;

	keyCodeMap[Ceng::KEYBOARD_KEY::LEFT_CTRL] = DIK_LCONTROL;
	keyCodeMap[Ceng::KEYBOARD_KEY::LEFT_ALT] = DIK_LALT;
	keyCodeMap[Ceng::KEYBOARD_KEY::RIGHT_ALT] = DIK_RALT;
	keyCodeMap[Ceng::KEYBOARD_KEY::RIGHT_CTRL] = DIK_RCONTROL;

	keyCodeMap[Ceng::KEYBOARD_KEY::UP_ARROW] = DIK_UP;
	keyCodeMap[Ceng::KEYBOARD_KEY::DOWN_ARROW] = DIK_DOWN;
	keyCodeMap[Ceng::KEYBOARD_KEY::LEFT_ARROW] = DIK_LEFT;
	keyCodeMap[Ceng::KEYBOARD_KEY::RIGHT_ARROW] = DIK_RIGHT;

	keyCodeMap[Ceng::KEYBOARD_KEY::PAGE_UP] = DIK_PGUP;
	keyCodeMap[Ceng::KEYBOARD_KEY::PAGE_DOWN] = DIK_PGDN;
	keyCodeMap[Ceng::KEYBOARD_KEY::HOME] = DIK_HOME;
	keyCodeMap[Ceng::KEYBOARD_KEY::END] = DIK_END;

	keyCodeMap[Ceng::KEYBOARD_KEY::LEFT_OS] = DIK_LWIN;
	keyCodeMap[Ceng::KEYBOARD_KEY::RIGHT_OS] = DIK_RWIN;

	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_1] = DIK_NUMPAD1;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_2] = DIK_NUMPAD2;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_3] = DIK_NUMPAD3;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_4] = DIK_NUMPAD4;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_5] = DIK_NUMPAD5;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_6] = DIK_NUMPAD6;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_7] = DIK_NUMPAD7;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_8] = DIK_NUMPAD8;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_9] = DIK_NUMPAD9;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_0] = DIK_NUMPAD0;

	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_PLUS] = DIK_NUMPADPLUS;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_MINUS] = DIK_NUMPADMINUS;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_STAR] = DIK_NUMPADSTAR;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_SLASH] = DIK_NUMPADSLASH;
	keyCodeMap[Ceng::KEYBOARD_KEY::NUMPAD_PERIOD] = DIK_NUMPADPERIOD;
}
		

Keyboard_DirectInput::Keyboard_DirectInput()
{
	core = nullptr;
	keyboard = nullptr;
}

Keyboard_DirectInput::~Keyboard_DirectInput()
{
	keyboard->Release();
	core->Release();
}

void Keyboard_DirectInput::Release()
{
	delete this;
}

const Ceng::UINT32 Keyboard_DirectInput::IsPressed(const Ceng::KEYBOARD_KEY::value key)
{
	Ceng::INT32 result;

	result = keyBuffer[keyCodeMap[key]];
	
	if (result && 0x80)
	{
		return 1;
	}

	return 0;
}

const CRESULT Keyboard_DirectInput::Restore()
{
	HRESULT hresult;

	// Read complete keyboard status
	hresult = keyboard->GetDeviceState(256,keyBuffer);
	if (hresult == DI_OK)
	{
		return CE_OK;
	}
	else if (hresult == DIERR_NOTACQUIRED || hresult == DIERR_INPUTLOST)
	{
		hresult = keyboard->Acquire();

		// Check if an another application has
		// keyboard focus
		if (hresult == DIERR_OTHERAPPHASPRIO)
		{
			return CE_OK;
		}

		// If the application has focus, wait until
		// the keyboard is acquired
		while (hresult != DI_OK)
		{
			hresult = keyboard->Acquire();
		};

		hresult = keyboard->GetDeviceState(256,keyBuffer);

		return CE_OK;		
	}

	return CE_ERR_FAIL;	
}