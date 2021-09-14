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
	bool moved;
};

enum MappedButtons {
	MOUSE_RIGHT,
	MOUSE_LEFT,
};

class InputHandler{
	public:
		static InputHandler *Instance(){
			if(instance == nullptr){
				instance = new InputHandler();
			}
			return instance;
		}

		static void destroy() {
			delete InputHandler::Instance();
		}

		inline void setMouseState(int x, int y){
			if(x != mouseState.x || y != mouseState.y)
				mouseState.moved = true;
			else
				mouseState.moved = false;
			mouseState.dx = x - mouseState.x;
			mouseState.dy = y - mouseState.y;
			mouseState.x = x;
			mouseState.y = y;
		}

		inline void setKeyValue(MappedButtons mb, bool isDown){
			buttons[mb] = isDown;
		}

		inline bool isKeyDown(MappedButtons mb){
			if(buttons.find(mb) == buttons.end()) return false;
			else return buttons[mb];
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

		InputHandler() {
			mouseState.dx = 0;
			mouseState.dy = 0;
			mouseState.x = 0;
			mouseState.y = 0;
			mouseState.yOff = 0;
			mouseState.moved = false;
		};

		static InputHandler* instance;
		std::unordered_map<MappedButtons, bool> buttons;
		MouseState mouseState;

};
typedef InputHandler _InputHandler;

#endif
