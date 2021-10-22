#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	if(argc < 3){
		std::cout << "USAGE:" << std::endl << args[0] << " query-mesh db-path" << std::endl;
		return 1;
	}
	std::string meshPath = args[1];
	std::string dbPath = args[2];
	if(!std::filesystem::exists(meshPath)){
		std::cout << "Error while reading file at: " << meshPath << std::endl;
		return 1;
	}
	std::filesystem::path featsPath = dbPath;
	featsPath /= "feats.csv";
	if(!std::filesystem::exists(featsPath)){
		std::cout << "Could not find " << featsPath << ".\nRun FeaturesExtractor on the mesh DB to generate the feature file first" << std::endl;
		return 1;
	}

	Mesh mesh(meshPath);
	std::cout << "Computing features of query mesh..." << std::endl;
	mesh.computeFeatures(Descriptors::descriptor_all & ~Descriptors::descriptor_diameter);
	mesh.getConvexHull()->computeFeatures(Descriptors::descriptor_diameter);

	std::cout << "Computing distances... " << std::endl;

	return 0;
}
