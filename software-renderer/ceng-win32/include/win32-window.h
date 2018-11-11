/*****************************************************************************
*
* win32-window.h
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_WIN32_WINDOW_H
#define _CONCEPT2_WIN32_WINDOW_H

#include <ceng/datatypes/window.h>

namespace Ceng
{

	#ifdef UNICODE
		typedef Ceng::StringUtf16 SystemString;
	#else
		typedef Ceng::StringUtf8 SystemString;
	#endif

	class WinProcThunk;

	class WinAPI_Window : PlatformWindow
	{
	protected:

		HWND windowHandle;
		
		WNDCLASSEX windowClass;

		SystemString *className;

		SystemString *caption,captionExtra;
		SystemString *fullCaption;

		WinProcThunk *thunk;
		
		
		// Location of the main window
		int screenLocX,screenLocY;

		// Size of the main window's client area
		int clientWidth,clientHeight;	
		
		int statusFlags;

		bool resized; // Set to true if windows size changes
		

		WinAPI_Window();

	public:

		WinAPI_Window(SystemString *caption,SystemString *fullCaption,
							 WNDCLASSEX *windowClass,SystemString *className,
							 WinProcThunk *thunk);

		void Configure(SystemString *caption,SystemString *fullCaption,
						HWND windowHandle,
						WNDCLASSEX *windowClass,SystemString *className,
						int statusFlags,
						const Ceng::UINT32 locX,const Ceng::UINT32 locY,
						const Ceng::UINT32 width,const Ceng::UINT32 height);
		
		virtual ~WinAPI_Window();

		virtual void Release() override;

		virtual void Show();
		virtual void Hide();

		virtual void Minimize();
		virtual void Maximize();

		virtual const Ceng::BOOL IsDestroyed();
		virtual const Ceng::BOOL IsActive();
		virtual const Ceng::BOOL IsResized();
		virtual const Ceng::BOOL IsVisible();

		virtual const CRESULT SetCaption(const Ceng::String &text);
		virtual const CRESULT SetCaptionExtra(const Ceng::String &extra);

		virtual void GetClientArea(Ceng::UINT32 *widthPtr,Ceng::UINT32 *heightPtr);

		virtual const CRESULT ProcessMessages();

		HWND GetWindowHandle();

		virtual const CRESULT Redraw();

		LRESULT WindowProc(::HWND handle,::UINT message,::WPARAM wParam,::LPARAM lParam);
	};

	

}

#endif