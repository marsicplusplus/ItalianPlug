#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	if(argc > 1) {
		std::cout << "USAGE:\n";
		std::cout << "\tItalianPlug Multimedia Retrieval System\t";
		std::cout << "Available binaries:\n";
		std::cout << "- FeaturesExtractor \t Extract features from all the meshes in a database" << std::endl;
		std::cout << "- StatsExtractir \t Extract statistics from all the meshes in a database" << std::endl;
		std::cout << "- Normalizer \t Normalize a single shape to a target number of vertices" << std::endl;
		return 0;
	}

	Renderer rend(1024, 720, "RendererGL");
	rend.initSystems();
	rend.start();
	return 0;
}
