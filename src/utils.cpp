#include "utils.hpp"
#include "glm/geometric.hpp"
#include <filesystem>

namespace Stats {
	std::string getParentFolderName(std::string filePath) {
		size_t found;
		std::cout << "Splitting: " << filePath << std::endl;
		found = filePath.find_last_of("/\\");
		auto folderPath = filePath.substr(0, found);
		found = folderPath.find_last_of("/\\");
		return folderPath.substr(found + 1);
	}

	ModelStatistics getModelStatistics(std::string modelFilePath) {
		//Assimp::Importer importer;

		//const aiScene* scene = importer.ReadFile(modelFilePath, aiProcess_GenBoundingBoxes);
		//if (!scene) {
			//// TODO: LOG ERROR
			//return ModelStatistics{};
		//}

		//const aiMesh* paiMesh = scene->mMeshes[0];
		//const auto primitiveTypes = paiMesh->mPrimitiveTypes;
		//std::string faceType = "";
		//if (primitiveTypes & aiPrimitiveType_TRIANGLE && primitiveTypes & aiPrimitiveType_POLYGON) {
			//faceType = "Both";
		//}
		//else if (primitiveTypes & aiPrimitiveType_TRIANGLE) {
			//faceType = "Only Triangles";
		//}
		//else if (primitiveTypes & aiPrimitiveType_POLYGON) {
			//faceType = "Only Polygons";
		//}
		//std::cout << modelFilePath << std::endl;
		//const auto classType = getParentFolderName(modelFilePath);
		//ModelStatistics modelStats = ModelStatistics{
			//classType,
			//paiMesh->mNumVertices,
			//paiMesh->mNumFaces,
			//faceType,
			//glm::vec3(paiMesh->mAABB.mMin.x, paiMesh->mAABB.mMin.y, paiMesh->mAABB.mMin.z),
			//glm::vec3(paiMesh->mAABB.mMax.x, paiMesh->mAABB.mMax.y, paiMesh->mAABB.mMax.z)
		//};

		return ModelStatistics{};
	}

	void getDatabaseStatistics(std::string databasePath) {

		std::ofstream myfile;
		myfile.open("example.csv");

		myfile <<
			"Class Type" << "," <<
			"Number of Vertices" << "," <<
			"Number of Faces" << "," <<
			"Types of Faces" << "," <<
			"Min Bounding Box: X" << "," <<
			"Min Bounding Box: Y" << "," <<
			"Min Bounding Box: Z" << "," <<
			"Max Bounding Box: X" << "," <<
			"Max Bounding Box: Y" << "," <<
			"Max Bounding Box: Z" << std::endl;

		std::string offExt(".off");
		std::string plyExt(".ply");
		for (auto& p : std::filesystem::recursive_directory_iterator(databasePath))
		{
			std::string extension = p.path().extension().string();
			if (extension == offExt || extension == plyExt) {
				ModelStatistics modelStats = getModelStatistics(p.path().string());
				myfile <<
					modelStats.classType << "," <<
					modelStats.numVertices << "," <<
					modelStats.numFaces << "," <<
					modelStats.faceType << "," <<
					modelStats.minBoundingBox.x << "," <<
					modelStats.minBoundingBox.y << "," <<
					modelStats.minBoundingBox.z << "," <<
					modelStats.maxBoundingBox.x << "," <<
					modelStats.maxBoundingBox.y << "," <<
					modelStats.maxBoundingBox.z << std::endl;
			}
		}
		myfile.close();
	}
}
OptionsMap* OptionsMap::instance = nullptr;
