#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"
#include "descriptors.hpp"
#include "shape_retriever.hpp"

int main(int argc, char* args[]) {
	if (argc < 3) {
		std::cout << "USAGE:" << std::endl << args[0] << " query-mesh db-path" << std::endl;
		return 1;
	}
	std::string meshPath = args[1];
	std::string dbPath = args[2];
	if (!std::filesystem::exists(meshPath)) {
		std::cout << "Error while reading file at: " << meshPath << std::endl;
		return 1;
	}

	Mesh mesh(meshPath);
	std::cout << "Computing features of query mesh..." << std::endl;
	mesh.computeFeatures(Descriptors::descriptor_all & ~Descriptors::descriptor_diameter);
	mesh.getConvexHull()->computeFeatures(Descriptors::descriptor_diameter);

	const auto mesh_ptr = std::make_shared<Mesh>(mesh);
	Retriever::retrieveSimiliarShapes(mesh_ptr, dbPath);
	const auto similarShapes = mesh_ptr->getSimilarShapes();
	if (!similarShapes.empty()) {
		std::cout << "Most similar shapes are... " << std::endl;
		std::cout << similarShapes[0].first << ":" << similarShapes[0].second << std::endl;
		std::cout << similarShapes[1].first << ":" << similarShapes[1].second << std::endl;
		std::cout << similarShapes[2].first << ":" << similarShapes[2].second << std::endl;
		std::cout << similarShapes[3].first << ":" << similarShapes[3].second << std::endl;
		std::cout << similarShapes[4].first << ":" << similarShapes[4].second << std::endl;
	}
	else {
		std::cout << "Error retrieving similar shapes. Returned vector was empty!" << std::endl;
		return 1;
	}


	return 0;
}

