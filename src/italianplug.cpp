#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	bool help = false;
	bool dirStats = false;
	bool normalizeMesh = false;
	int targetVerts = 0;
	std::string meshPath;
	std::string dirPath;
	for(int i = 1; i < argc; i++){
		if(strncmp(args[i], "--help", strlen("--help")) == 0){
			help = true;
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
		if(strncmp(args[i], "--normalize-mesh", strlen("--normalize-mesh")) == 0){
			if (i + 2 <= argc) {
				normalizeMesh = true;
				meshPath = args[i + 1];
				targetVerts = std::strtol(args[i + 2], nullptr, 0);
			}
			else {
				std::cerr << "ERROR: it seems like you forgot to pass me the path of the Database\n" << std::flush;
				help = true;
			}
		}
	}
	if(help) {
		std::cout << "USAGE:\n";
		std::cout << args[0] << " [options]\n";
		std::cout << "Available options:\n";
		std::cout << "--help \t\t Show this message\n";
		std::cout << "--dir-stats path/to/db/ \t Recursively analyze and print the stats of every single mesh found in the passed directory\n";
		return 0;
	}
	if(dirStats){
		Stats::getDatabaseStatistics(dirPath);
	} else if (normalizeMesh) {
		Mesh mesh(meshPath);
		mesh.normalize(targetVerts);
		mesh.writeMesh();
	} else {
		Renderer rend(1024, 720, "RendererGL");
		rend.initSystems();
		rend.start();
	}
	return 0;
}
