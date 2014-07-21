#pragma once 

#include <Windows.h>
#include <WindowsX.h>
#include "Controls.h"
#include "D3DGraphics.h"

enum Errors
{
	APP_NO_WINDOW =1
};

class App
{
	public:
		App();
		~App();

		bool Setup(LPCSTR appname, int width, int height, bool fullscreen, bool wireframe, int cmd, HINSTANCE instance);

		LPCSTR GetError(int);
		inline HWND GetHWND() { return m_pHwnd; }
		inline int GetLastError() { return m_pAppError; }
		int Run();
		
	private:
		HWND m_pHwnd;
		int m_pAppError;
		int m_WindowHeight;
		int m_WindowWidth;
		inline void SetLastError(int error) { m_pAppError = error; }

}; extern App Application;
