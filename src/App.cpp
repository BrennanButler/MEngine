#include "App.h"

App Application;

App::App()
{
	

}

App::~App()
{
}

bool App::Setup(LPCSTR appname, int width, int height, bool fullscreen, bool wireframe, int cmd, HINSTANCE instance)
{
	m_pHwnd = CreateWindowEx(WS_EX_APPWINDOW, appname, appname, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, instance, NULL);
	if(m_pHwnd == NULL)
		return false;
	ShowWindow(m_pHwnd, cmd);
	UpdateWindow(m_pHwnd);
	m_WindowHeight = height;
	m_WindowWidth = width;
	return Graphics.RunDirectX(m_pHwnd, width, height, fullscreen, wireframe);
}

LPCSTR App::GetError(int errorcode)
{
	switch(errorcode)
	{
		case APP_NO_WINDOW:
			return "Application could not create window";
	}
	return "null";
}

int App::Run()
{
	MSG msg = {0};

	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			Graphics.UpdateScene();
			Graphics.Render();
		}
	}

	Graphics.CleanUp();

	return static_cast<int>(msg.wParam);
}