#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "glad/glad.h"
#include "glm/vec3.hpp"
#include <any>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <Eigen/Core>

#define DESCRIPTORS_NUM 56
typedef unsigned char BYTE;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
};

struct ModelStatistics {
	std::string classType;
	unsigned int numVertices;
	unsigned int numFaces;
	std::string faceType;
	float centroidDistance;
	float longestEdge;
	glm::vec3 minBoundingBox;
	glm::vec3 maxBoundingBox;
	float angleFirstToX;
	float angleSecondToY;
};

typedef unsigned char BYTE;
typedef std::pair<int, std::string> IndexNamePair;
typedef std::unordered_map<std::string, std::vector<IndexNamePair>> ClassToMeshIndexNameMap;

namespace Importer {
	bool importModel(
		const std::filesystem::path& filePath, 
		Eigen::MatrixXf& V,
		Eigen::MatrixXi& F
	);
};

namespace Exporter {
	bool exportModel(
		const std::filesystem::path& filePath, 
		const Eigen::MatrixXf& V, 
		const Eigen::MatrixXi& F
	);
}

template<class Iter_T, class Iter2_T, class Iter3_T>
inline double vectorDistance(Iter_T first, Iter_T last, Iter2_T first2, Iter3_T weights) {
	double ret = 0.0;
	while (first != last) {
		double dist = (*first++) - (*first2++);
		ret += dist * dist * *weights++;
	}
	return ret > 0.0 ? sqrt(ret) : 0.0;
}

namespace Stats {
	ModelStatistics getModelStatistics(std::string modelFilePath);
	void getDatabaseStatistics(std::string databasePath, std::string fp = "stats.csv");
	void getDatabaseFeatures(std::string dbPath);
};

namespace FeatureVector {
	void getFeatureVectorClassToIndicesName(std::filesystem::path dbPath, Eigen::VectorXd& dbFeatureVector, ClassToMeshIndexNameMap& classTypeToIndicesNames, int& origDimensionality, int& numOfDataPoints);
	void formatReducedFeatureVector(std::vector<double> flatFeatureVectors, int numOfDataPoints, Eigen::MatrixXd& reducedFeatureVectors);
};

enum Options{
	DRAW_MODE,
};

enum Features{
	FEAT_AREA_3D 		= 0,
	FEAT_MVOLUME_3D,
	FEAT_BBVOLUME_3D,
	FEAT_DIAMETER_3D,
	FEAT_COMPACTNESS_3D,
	FEAT_ECCENTRICITY_3D,
	FEAT_A3_3D,
	FEAT_D1_3D,
	FEAT_D2_3D,
	FEAT_D3_3D,
	FEAT_D4_3D,
};

enum DrawMode {
	WIREFRAME = 0,			//GL_LINE
	POINT_CLOUD, 			//GL_POINTS
	SHADED_MESH, 			//GL_FILL
	SHADED_MESH_WIREFRAME, 	//GL_FILL and GL_LINE
	DRAW_MODES
};

class OptionsMap{
	public:
		static OptionsMap *Instance(){
			if(instance == nullptr){
				instance = new OptionsMap();
			}
			return instance;
		}

		inline void destroy(){
			delete instance;
		}

		inline void setOption(Options opt, int v){
			opts[opt] = v;
		}

		inline int getOption(Options opt){
			return opts[opt];
		}

	private:
		std::unordered_map<Options, int> opts;

		OptionsMap(){
			opts[DRAW_MODE] = POINT_CLOUD;
		};

		~OptionsMap(){};

		static OptionsMap* instance;
};

#endif
