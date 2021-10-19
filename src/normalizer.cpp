#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	if(argc < 3){
		std::cout << "USAGE:" << std::endl << args[0] << " path-to-mesh target-vertices" << std::endl;
		return 1;
	}
	std::string meshPath = args[1];
	int targetVerts = std::strtol(args[2], nullptr, 0);
	Mesh mesh(meshPath);
	mesh.normalize(targetVerts);
	mesh.writeMesh();
	return 0;
}
