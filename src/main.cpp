#include "renderer.hpp"
#include "utils.hpp"
#include "spdlog/spdlog.h"

int main(int argc, char* args[]) {
	spdlog::set_level(spdlog::level::info); // Set global log level to debug
	spdlog::info("Starting {} with {} arguments", args[0], argc);
	bool help = false;
	bool meshStats = false;
	bool dirStats = false;
	bool hasMesh = false;
	std::string meshPath;
	for(int i = 1; i < argc; i++){
		if(strncmp(args[i], "--help", strlen("--help")) == 0){
			help = true;	
		}
		if(strncmp(args[i], "--preload-mesh", strlen("--preload-mesh")) == 0){
			if(i + 1 < argc){
				hasMesh = true;
				meshPath = args[i+1];
			} else {
				spdlog::error("ERROR: it seems like you forgot to pass me the path of the mesh!");
				help = true;
				break;
			}
		}
	}
	if(help) {
		std::cout << "USAGE:\n";
		std::cout << args[0] << " [options]\n";
		std::cout << "Available options:\n";
		std::cout << "--help \t\t Show this message\n";
		std::cout << "--preload-mesh path/to/mesh \t Start the renderer with a preloaded mesh\n";
		std::cout << "--mesh-stats path/to/mesh \t Analyze and print the stats of a single mesh\n";
		std::cout << "--dir-stats path/to/meshdir/ \t Recursively analyze and print the stats of every single mesh found in the passed directory\n";
		return 0;
	}
	if(dirStats){
	} else if (meshStats) {
	} else {
		Renderer rend(1024, 720, "RendererGL");
		rend.initSystems();
		if(hasMesh) rend.setMesh(meshPath);
		rend.start();
	}
	return 0;
}
