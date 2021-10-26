#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"
#include <filesystem>

int main(int argc, char* args[]) {
	if(argc < 3){
		std::cout << "USAGE:" << std::endl << args[0] << " path-to-mesh target-vertices" << std::endl;
		return 1;
	}

	std::filesystem::path meshPath = args[1];
	int targetVerts = std::strtol(args[2], nullptr, 0);

	Mesh mesh(meshPath);
	mesh.normalize(targetVerts);

	const auto meshName = meshPath.filename();
	const auto meshClass = meshPath.parent_path().filename();
	auto origDBDir = meshPath.parent_path().parent_path();
	std::filesystem::path writePath = origDBDir.replace_filename("NormalizedDB") / meshClass / meshName;
	std::filesystem::create_directories(writePath.parent_path());
	mesh.writeMesh(writePath);
	return 0;
}
