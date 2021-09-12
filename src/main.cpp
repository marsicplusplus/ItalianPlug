#include "renderer.hpp"
#include "utils.hpp"

int main() {
	Renderer rend(1024, 720, "RendererGL");
	rend.initSystems();
	rend.start();
	return 0;
}
