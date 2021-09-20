#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	bool help = false;
	bool meshStats = false;
	bool dirStats = false;
	bool hasMesh = false;
	std::string meshPath;
	std::string dirPath;
	for(int i = 1; i < argc; i++){
		if(strncmp(args[i], "--help", strlen("--help")) == 0){
			help = true;	
		}
		if(strncmp(args[i], "--preload-mesh", strlen("--preload-mesh")) == 0){
			if(i + 1 <= argc){
				hasMesh = true;
				meshPath = args[i+1];
			} else {
				std::cerr << "ERROR: it seems like you forgot to pass me the path of the mesh!\n" << std::flush;
				help = true;
			}
		}
		if (strncmp(args[i], "--dir-stats", strlen("--dir-stats")) == 0) {
			if (i + 1 <= argc) {
				dirStats = true;
				dirPath = args[i + 1];
			}
			else {
				std::cerr << "ERROR: it seems like you forgot to pass me the path of the database!\n" << std::flush;
				help = true;
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
		Stats::getDatabaseStatistics(dirPath);
	} else if (meshStats) {
		Stats::getModelStatistics(meshPath);
	} else {
		Renderer rend(1024, 720, "RendererGL");
		rend.initSystems();
		if(hasMesh) rend.setMesh(meshPath);
		rend.start();
	}
	return 0;
}
