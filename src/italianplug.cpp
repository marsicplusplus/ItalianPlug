#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	std::string meshPath;
	if(argc > 1) {
		if(strncmp("--help", args[1], strlen("--help")) == 0) {
			std::cout << "USAGE:\n";
			std::cout << "\tItalianPlug Multimedia Retrieval System\t";
			std::cout << "Available binaries:\n";
			std::cout << "- FeaturesExtractor \t Extract features from all the meshes in a database" << std::endl;
			std::cout << "- StatsExtractir \t Extract statistics from all the meshes in a database" << std::endl;
			std::cout << "- Normalizer \t\t Normalize a single shape to a target number of vertices" << std::endl;
			std::cout << "- ItalianPlug \t\t Mesh Visualizer and UI. Can be launched with a mesh preloaded by using:\n\t\t\t\t ./ItalianPlug path-to-mesh" << std::endl;
			return 0;
		}
	}

	Renderer rend(1024, 720, "RendererGL");
	rend.initSystems();
	if(std::filesystem::exists(args[1])){
		rend.setMesh(args[1]);
	}
	rend.start();
	return 0;
}
