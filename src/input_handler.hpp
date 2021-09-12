#ifndef __INPUT_HANDLER_HPP__
#define __INPUT_HANDLER_HPP__

#include <algorithm>
#include <map>

struct MouseState {
	int dx;
	int x;
	int dy;
	int y;
	double yOff;
	bool leftDown;
	bool rightDown;
};

class InputHandler{
	public:
		static InputHandler *Instance(){
			if(instance == nullptr){
				instance = new InputHandler();
			}
			return instance;
		}

		inline void init() {
			mouseState.dx = 0;
			mouseState.dy = 0;
			mouseState.x = 0;
			mouseState.y = 0;
			mouseState.yOff = 0;
			mouseState.rightDown = false;
			mouseState.leftDown = false;
		}

		inline void setMouseState(int x, int y){
			mouseState.dx = x - mouseState.x;
			mouseState.dy = y - mouseState.y;
			mouseState.x = x;
			mouseState.y = y;
		}

		inline void setLeftDown(bool v){
			mouseState.leftDown = v;
		}

		inline void setRightDown(bool v){
			mouseState.rightDown = v;
		}

		inline MouseState getMouseState() const {
			return mouseState;
		}

		inline void scrollState(float yOff){
			mouseState.yOff = yOff;
		}
		inline float getScrollState(){
			return mouseState.yOff;
		}
	private:
		MouseState mouseState;

		InputHandler() {};
		~InputHandler() {};

		static InputHandler* instance;

};
typedef InputHandler _InputHandler;

#endif
