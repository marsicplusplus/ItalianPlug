#include "utils.hpp"
#include "glm/geometric.hpp"
#include <filesystem>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "igl/writeOFF.h"
#include "igl/writePLY.h"
#include "igl/readOFF.h"
#include "igl/readPLY.h"


namespace Importer {
	bool importModel(const std::filesystem::path& filePath, Eigen::MatrixXf& V, Eigen::MatrixXi& F) {

		// First try to import the model using libigl if that fails then we try with assimp
		bool modelImportedWithLibigl = false;
		if (filePath.extension() == ".ply") {
			modelImportedWithLibigl = igl::readPLY(filePath.string(), V, F);
		} else if (filePath.extension() == ".off") {
			modelImportedWithLibigl = igl::readOFF(filePath.string(), V, F);
		} else {
			// TODO: LOG ERROR
			return false;
		}

		// Ensure that the model has only triangles
		if (modelImportedWithLibigl && F.cols() == 3) {
			return true;
		}

		// Loading an off or ply file failed with libigl (most likely because there was a combination of triangles and polygons)
		// If we load a file that only has polygons then we should reload it with assimp
		// Load using assimp with the aiProcess_Triangulate option to convert all polygons to triangles
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filePath.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality | aiProcess_SortByPType);
		if (!scene) {
			// TODO: LOG ERROR
			return false;
		}

		const aiMesh* paiMesh = scene->mMeshes[0];

		// Resize the Vertices and Faces Matrices using the info from assimp
		V.conservativeResize(paiMesh->mNumVertices, 3);
		F.conservativeResize(paiMesh->mNumFaces, 3);

		// Populare the Vertices matrix
		for (int vertexIndex = 0; vertexIndex < paiMesh->mNumVertices; vertexIndex++) {
			const aiVector3D* pPos = &(paiMesh->mVertices[vertexIndex]);
			V.row(vertexIndex) = Eigen::Vector3f((float)pPos->x, (float)pPos->y, (float)pPos->z);
		}

		// Populare the Faces matrix
		for (int faceIndex = 0; faceIndex < paiMesh->mNumFaces; faceIndex++) {
			const aiFace& Face = paiMesh->mFaces[faceIndex];
			assert(Face.mNumIndices == 3);
			F.row(faceIndex) = Eigen::Vector3i(
				Face.mIndices[0],
				Face.mIndices[1],
				Face.mIndices[2]
			);
		}

		return true;
	}
}

namespace Exporter {
	bool exportModel(const std::filesystem::path& filePath, const Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
		bool modelExported = false;
		if (filePath.extension() == ".ply") {
			modelExported = igl::writePLY(filePath.string(), V, F);
		} else if (filePath.extension() == ".off") {
			modelExported = igl::writeOFF(filePath.string(), V, F);
		}

		return modelExported;
	}
}

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
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(modelFilePath, aiProcess_GenBoundingBoxes);
		if (!scene) {
			// TODO: LOG ERROR
			std::cerr << "Could not read mesh file at " << modelFilePath << std::endl;
			return ModelStatistics{};
		}

		const aiMesh* paiMesh = scene->mMeshes[0];
		const auto primitiveTypes = paiMesh->mPrimitiveTypes;
		std::string faceType = "";
		if (primitiveTypes & aiPrimitiveType_TRIANGLE && primitiveTypes & aiPrimitiveType_POLYGON) {
			faceType = "Both";
		}
		else if (primitiveTypes & aiPrimitiveType_TRIANGLE) {
			faceType = "Only Triangles";
		}
		else if (primitiveTypes & aiPrimitiveType_POLYGON) {
			faceType = "Only Polygons";
		}
		std::cout << modelFilePath << std::endl;
		const auto classType = getParentFolderName(modelFilePath);
		ModelStatistics modelStats = ModelStatistics{
			classType,
			paiMesh->mNumVertices,
			paiMesh->mNumFaces,
			faceType,
			glm::vec3(paiMesh->mAABB.mMin.x, paiMesh->mAABB.mMin.y, paiMesh->mAABB.mMin.z),
			glm::vec3(paiMesh->mAABB.mMax.x, paiMesh->mAABB.mMax.y, paiMesh->mAABB.mMax.z)
		};

		return modelStats;
	}

	void getDatabaseStatistics(std::string databasePath, std::string filePath) {

		std::ofstream myfile;
		myfile.open(filePath);

		myfile <<
			"Filename" << "," <<
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
					p.path().string() << "," << 
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
