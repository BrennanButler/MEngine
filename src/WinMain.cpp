/*
	What's needed:

		Loading and caching game data
		Interface/GUI 
		Event managment
		Scripting?
		Audio
		Physics
		Network/multiplayer
		AI
		Debugging
		Memory managment
		3D & 2D rendering
*/

#include "App.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_ESCAPE)
		DestroyWindow(hwnd);
	switch(msg)
	{
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "MEngine";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Failed to register window class", "ERROR", MB_OK);
		return 0;
	}

	if(!Application.Setup("MEngine", 600, 600, true, false, nCmdShow, hInstance))
	{
		MessageBox(NULL, "Failed to create window", "ERROR", MB_OK);
		return 0;
	}

	MessageBox(NULL, "There is a bug in which you'll need to end this process in the task manager.", "bug", MB_OK);

	return Application.Run();
}