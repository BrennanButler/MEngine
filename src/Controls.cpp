#include "Controls.h"

Controls::Keyboard::Keyboard() 
{
}

Controls::Keyboard::~Keyboard() {}

int Controls::Keyboard::OnKeyPressed(int key)
{
	return 1;
}

void Controls::Keyboard::ToggleKey(int key)
{
	keys[key] = true;
	OnKeyPressed(key);
}

bool Controls::Keyboard::IsKeyDown(int key)
{
	if(keys[key]) return true;
	return false;
}

Controls::Mouse::Mouse() {}
Controls::Mouse::~Mouse() {}

void Controls::Mouse::SetClickLocation(long x, long y)
{
	MouseX = x;
	MouseY = y;
}

void Controls::Mouse::GetClickLocation(long& x, long& y)
{
	x = MouseX;
	y = MouseY;
}