#include "utils.hpp"
#include "glm/geometric.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "spdlog/spdlog.h"

namespace Loader {
	bool loadOFF(std::string fileName, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices) {
		std::vector<Vertex> tmp;
		std::ifstream offFile;

		offFile.open(fileName);
		if(!offFile){
			spdlog::error("loadOFF::Cannot load OFF file at: {}", fileName);
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
		const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality);
		if(!scene){
			// TODO: LOG ERROR
			return false;
		}
		const aiMesh *paiMesh = scene->mMeshes[0];
		spdlog::info("Loaded {}", fileName);
		spdlog::info("Num Vertices: {}", paiMesh->mNumVertices);
		spdlog::info("Num Faces: {}", paiMesh->mNumFaces);
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

OptionsMap* OptionsMap::instance = nullptr;
