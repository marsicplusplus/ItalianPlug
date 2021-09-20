#include "utils.hpp"
#include "glm/geometric.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <filesystem>

namespace Loader {
	bool loadOFF(std::string fileName, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices) {
		std::vector<Vertex> tmp;
		std::ifstream offFile;

		offFile.open(fileName);
		if(!offFile){
			std::cerr << "loadOFF::Cannot load OFF file at: " << fileName << std::endl;
			return false;
		}
		auto skipComments = [](std::ifstream& file, std::string &line) {
			while(true){
				bool skip = false;
				for(auto &c : line) {
					if(c == ' ') continue;
					if(c == '#') {
						skip = true;
						break;
					}
					else 
						break;
				}
				if(!skip) break;
				std::getline(file, line);
			}	
		};

		std::string line;
		std::getline(offFile, line);
		skipComments(offFile, line);
		if(line == "OFF"){
			std::getline(offFile, line);
			skipComments(offFile, line);
		}
		std::istringstream iss(line);
		int nVertices, nIdx, nEdges;
		iss >> nVertices >> nIdx >> nEdges;

		std::getline(offFile, line);
		skipComments(offFile, line);
		for(int i = 0; i < nVertices; i++) {
			std::istringstream iss(line);
			float x, y, z;
			iss >> x >> y >> z;
			tmp.push_back(Vertex{
					{x, y, z},
					{.0f, .0f, .0f}
					});
			std::getline(offFile, line);
			skipComments(offFile, line);
		}

		for(int i = 0; i < nIdx; i++) {
			std::istringstream iss(line);
			int vertexPerFace;
			iss >> vertexPerFace;
			for(int j = 0; j < vertexPerFace; j++){
				unsigned int a;
				iss >> a;
				vertices.push_back(tmp[a]);
				//indices.push_back(vertices.size());
			}
			std::getline(offFile, line);
			skipComments(offFile, line);
		}

		for(int i = 0; i < nIdx - 3; i++) {
			Vertex &v1 = vertices[i * 3];
			Vertex &v2 = vertices[i * 3 + 1];
			Vertex &v3 = vertices[i * 3 + 2];
			glm::vec3 v1v2 = v2.pos - v1.pos;
			glm::vec3 v1v3 = v3.pos - v1.pos;
			glm::vec3 normal = glm::cross(v1v2, v1v3);
			normal = glm::normalize(normal);
			v1.normal = normal;
			v2.normal = normal;
			v3.normal = normal;
		}
		return true;
	}

	bool loadModel(std::string fileName, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices){
		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality | aiProcess_SortByPType);
		if(!scene){
			// TODO: LOG ERROR
			return false;
		}

		const aiMesh *paiMesh = scene->mMeshes[0];
		std::cout << fileName << std::endl;
		std::cout << "Num Vertices: " << paiMesh->mNumVertices << std::endl;
		std::cout << "Num Faces: " << paiMesh->mNumFaces << std::endl;
		for (int i = 0 ; i < paiMesh->mNumVertices ; i++) {
			const aiVector3D* pPos = &(paiMesh->mVertices[i]);
			const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
			Vertex v{
				glm::vec3(pPos->x, pPos->y, pPos->z),
				glm::vec3(pNormal->x, pNormal->y, pNormal->z)
			};
			vertices.push_back(v);
		}
		for (int i = 0 ; i < paiMesh->mNumFaces ; i++) {
			const aiFace& Face = paiMesh->mFaces[i];
			assert(Face.mNumIndices == 3);
			indices.push_back(Face.mIndices[0]);
			indices.push_back(Face.mIndices[1]);
			indices.push_back(Face.mIndices[2]);
		}
		return true;
	}
};

namespace Stats {
	std::string getParentFolderName(std::string filePath)
	{
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
