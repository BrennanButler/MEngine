#ifndef _CONTROLS_H_
#define	_CONTROLS_H_

//TODO: Find a better way of implementing controls. Right now the design of this namespace isn't very good.

#include <Windows.h>

namespace Controls
{
	class Keyboard {
	
		public:
			Keyboard();
			~Keyboard();

			void ToggleKey(int);

			int OnKeyPressed(int key);

			bool IsKeyDown(int key);

		private:
			bool keys[256];
	};

	class Mouse {
	
		public:
			Mouse();
			~Mouse();

		void SetClickLocation(long x, long y);

		void GetClickLocation(long& x, long& y);

		private:
			int MouseX, MouseY;
	
	};

	class Joystick {
		
		public:
			Joystick();
			~Joystick();
	
	};

	extern Keyboard kbd;
	extern Mouse mouse;
}

#endif