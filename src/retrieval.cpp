#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"
#include "descriptors.hpp"
#include "shape_retriever.hpp"

int main(int argc, char* args[]) {
	if (argc < 4) {
		std::cout << "USAGE:" << std::endl << args[0] << " query-mesh db-path n-shapes [ANN=true|false]" << std::endl;
		return 1;
	}
	std::string meshPath = args[1];
	std::string dbPath = args[2];
	int nShapes = atoi(args[3]);

	if (!std::filesystem::exists(meshPath)) {
		std::cout << "Error while reading file at: " << meshPath << std::endl;
		return 1;
	}

	Mesh mesh(meshPath);
	const auto mesh_ptr = std::make_shared<Mesh>(mesh);
	if(argc == 5 && strncmp(args[4], "ANN=true", strlen("ANN=true")) == 0)
		Retriever::retrieveSimiliarShapesKNN(mesh_ptr, dbPath, nShapes);
	else
		Retriever::retrieveSimiliarShapes(mesh_ptr, dbPath);
	const auto similarShapes = mesh_ptr->getSimilarShapes();
	if (!similarShapes.empty()) {
		std::cout << "Most similar shapes are... " << std::endl;
		for(auto i = 0; i < nShapes; ++i)
			std::cout << similarShapes[i].first << ":" << similarShapes[i].second << std::endl;
	}
	else {
		std::cout << "Error retrieving similar shapes. Returned vector was empty!" << std::endl;
		return 1;
	}


	return 0;
}

