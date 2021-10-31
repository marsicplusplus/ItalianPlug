#include "utils.hpp"
#include "normalization.hpp"
#include "glm/geometric.hpp"
#include <filesystem>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "igl/writeOFF.h"
#include "igl/writePLY.h"
#include "igl/readOFF.h"
#include "igl/readPLY.h"
#include "mesh.hpp"
#include "rapidcsv.h"
#include <future>
#include <mutex>


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
		Mesh mesh(modelFilePath);
		Eigen::Vector3f c;
		mesh.getCentroid(c);
		glm::vec3 bbMin(paiMesh->mAABB.mMin.x, paiMesh->mAABB.mMin.y, paiMesh->mAABB.mMin.z);
		glm::vec3 bbMax(paiMesh->mAABB.mMax.x, paiMesh->mAABB.mMax.y, paiMesh->mAABB.mMax.z);
		glm::vec3 res = bbMax - bbMin;

		const auto covarianceMatirx = Normalization::calculateCovarianceMatrix(mesh.getVertices(), c);
		const auto eigenResults = Normalization::calculateEigen(covarianceMatirx);
		const auto majorEigenVector = eigenResults[2].first;
		const auto minorEigenVector = eigenResults[1].first;

		const auto majorToX = std::abs(Normalization::angleBetween(majorEigenVector, Eigen::Vector3f(1, 0, 0)));
		const auto minorToY = std::abs(Normalization::angleBetween(minorEigenVector, Eigen::Vector3f(0, 1, 0)));

		ModelStatistics modelStats = ModelStatistics{
			classType,
				paiMesh->mNumVertices,
				paiMesh->mNumFaces,
				faceType,
				c.norm(),
				fmaxf(fmaxf(res.x, res.y), res.z),
				bbMin,
				bbMax,
				static_cast<float>(fmod(majorToX, 180.0f)),
				static_cast<float>(fmod(minorToY, 180.0f)),
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
			"Longest Edge" << "," << 
			"Centroid Distance" << "," << 
			"Min Bounding Box: X" << "," <<
			"Min Bounding Box: Y" << "," <<
			"Min Bounding Box: Z" << "," <<
			"Max Bounding Box: X" << "," <<
			"Max Bounding Box: Y" << "," <<
			"Max Bounding Box: Z" << "," <<
			"AABB Volume" << "," <<
			"Angle First Eigenvector To X" << "," <<
			"Angle Second Eigenvector To Y" << std::endl;

		std::string offExt(".off");
		std::string plyExt(".ply");
		for (auto& p : std::filesystem::recursive_directory_iterator(databasePath)){
			std::string extension = p.path().extension().string();
			if (extension == offExt || extension == plyExt) {
				ModelStatistics modelStats = getModelStatistics(p.path().string());
				//std::cout << "Extracting " << p.path().string() << std::endl << std::flush;
				glm::vec3 tmp = glm::abs((modelStats.maxBoundingBox - modelStats.minBoundingBox));
				myfile <<
					p.path().string() << "," << 
					modelStats.classType << "," <<
					modelStats.numVertices << "," <<
					modelStats.numFaces << "," <<
					modelStats.faceType << "," <<
					modelStats.longestEdge << "," <<
					modelStats.centroidDistance << "," << 
					modelStats.minBoundingBox.x << "," <<
					modelStats.minBoundingBox.y << "," <<
					modelStats.minBoundingBox.z << "," <<
					modelStats.maxBoundingBox.x << "," <<
					modelStats.maxBoundingBox.y << "," <<
					modelStats.maxBoundingBox.z << "," <<
					tmp.x * tmp.y * tmp.z << "," <<
					modelStats.angleFirstToX << "," <<
					modelStats.angleSecondToY << std::endl;
			}
		}
		myfile.close();
	}

	void getDatabaseFeatures(std::string dbPath){
		std::filesystem::path fp = dbPath;
		std::filesystem::path currPath = std::filesystem::current_path();
		std::filesystem::current_path(fp);
		std::mutex fileMutex;
		std::vector<std::future<void>> futures;
		std::vector<std::filesystem::path> classPaths;
		std::vector<DescriptorMap> features;
		std::vector<std::string> names;
		for (auto& p : std::filesystem::recursive_directory_iterator(".")) {
			if(p.is_directory()){
				classPaths.push_back(p.path());
			}
		}
		for(auto& cp : classPaths){
			futures.push_back(std::async(std::launch::async, [&cp, &names, &features, &fileMutex]{
				for(auto &p : std::filesystem::recursive_directory_iterator(cp)){
					std::string extension = p.path().extension().string();
					std::string offExt(".off");
					std::string plyExt(".ply");
					if (extension == offExt || extension == plyExt) {
						std::cout << "Compute features for " << p.path().string() << std::endl;
						Mesh mesh(p.path().string());
						mesh.computeFeatures(Descriptors::descriptor_all & 
								~Descriptors::descriptor_diameter);
						mesh.getConvexHull()->computeFeatures(Descriptors::descriptor_diameter);
						try{
							const std::lock_guard<std::mutex> lock(fileMutex);
							names.push_back(std::filesystem::absolute(p.path()).string());
							DescriptorMap dm = mesh.getDescriptorMap();
							dm[FEAT_DIAMETER_3D] = mesh.getConvexHull()->getDescriptor(FEAT_DIAMETER_3D);
							features.push_back(dm);
						} catch(std::bad_variant_access e){
							std::cout << "Error retrieving features for " << p.path().string() << ": " << e.what();
						}
					}
				}
			}));
		}
		for(auto &a : futures){
			a.get();
		}

		std::cout << "Normalization..." << std::endl;
		DescriptorMap avgs;
		DescriptorMap deviations;
		avgs[FEAT_AREA_3D] = 0.0f;
		avgs[FEAT_MVOLUME_3D] = 0.0f;
		avgs[FEAT_BBVOLUME_3D] = 0.0f;
		avgs[FEAT_DIAMETER_3D] = 0.0f;
		avgs[FEAT_COMPACTNESS_3D] = 0.0f; 
		avgs[FEAT_ECCENTRICITY_3D] = 0.0f;
		deviations[FEAT_AREA_3D] = 0.0f;
		deviations[FEAT_MVOLUME_3D] = 0.0f;
		deviations[FEAT_BBVOLUME_3D] = 0.0f;
		deviations[FEAT_DIAMETER_3D] = 0.0f;
		deviations[FEAT_COMPACTNESS_3D] = 0.0f; 
		deviations[FEAT_ECCENTRICITY_3D] = 0.0f;
		for(auto& a : features){
			avgs[FEAT_AREA_3D] = std::get<float>(avgs[FEAT_AREA_3D]) + std::get<float>(a[FEAT_AREA_3D]);
			avgs[FEAT_MVOLUME_3D] = std::get<float>(avgs[FEAT_MVOLUME_3D]) + std::get<float>(a[FEAT_MVOLUME_3D]);
			avgs[FEAT_BBVOLUME_3D] = std::get<float>(avgs[FEAT_BBVOLUME_3D]) + std::get<float>(a[FEAT_BBVOLUME_3D]);
			avgs[FEAT_DIAMETER_3D] = std::get<float>(avgs[FEAT_DIAMETER_3D]) + std::get<float>(a[FEAT_DIAMETER_3D]);
			avgs[FEAT_COMPACTNESS_3D] = std::get<float>(avgs[FEAT_COMPACTNESS_3D]) + std::get<float>(a[FEAT_COMPACTNESS_3D]);
			avgs[FEAT_ECCENTRICITY_3D] = std::get<float>(avgs[FEAT_ECCENTRICITY_3D]) + std::get<float>(a[FEAT_ECCENTRICITY_3D]);
		}
		avgs[FEAT_AREA_3D] = std::get<float>(avgs[FEAT_AREA_3D]) / features.size();
		avgs[FEAT_MVOLUME_3D] = std::get<float>(avgs[FEAT_MVOLUME_3D]) / features.size();
		avgs[FEAT_BBVOLUME_3D] = std::get<float>(avgs[FEAT_BBVOLUME_3D]) / features.size();
		avgs[FEAT_DIAMETER_3D] = std::get<float>(avgs[FEAT_DIAMETER_3D]) / features.size();
		avgs[FEAT_COMPACTNESS_3D] = std::get<float>(avgs[FEAT_COMPACTNESS_3D]) / features.size();
		avgs[FEAT_ECCENTRICITY_3D] = std::get<float>(avgs[FEAT_ECCENTRICITY_3D]) / features.size();

		for(auto& a : features){
			deviations[FEAT_AREA_3D] = std::get<float>(deviations[FEAT_AREA_3D]) + std::pow(std::get<float>(a[FEAT_AREA_3D]) - std::get<float>(avgs[FEAT_AREA_3D]), 2.0f);
			deviations[FEAT_MVOLUME_3D] = std::get<float>(deviations[FEAT_MVOLUME_3D]) + std::pow(std::get<float>(a[FEAT_MVOLUME_3D]) - std::get<float>(avgs[FEAT_MVOLUME_3D]), 2.0f);
			deviations[FEAT_BBVOLUME_3D] = std::get<float>(deviations[FEAT_BBVOLUME_3D]) + std::pow(std::get<float>(a[FEAT_BBVOLUME_3D]) - std::get<float>(avgs[FEAT_BBVOLUME_3D]), 2.0f);
			deviations[FEAT_DIAMETER_3D] = std::get<float>(deviations[FEAT_DIAMETER_3D]) + std::pow(std::get<float>(a[FEAT_DIAMETER_3D]) - std::get<float>(avgs[FEAT_DIAMETER_3D]), 2.0f);
			deviations[FEAT_COMPACTNESS_3D] = std::get<float>(deviations[FEAT_COMPACTNESS_3D]) + std::pow(std::get<float>(a[FEAT_COMPACTNESS_3D]) - std::get<float>(avgs[FEAT_COMPACTNESS_3D]), 2.0f);
			deviations[FEAT_ECCENTRICITY_3D] = std::get<float>(deviations[FEAT_ECCENTRICITY_3D]) + std::pow(std::get<float>(a[FEAT_ECCENTRICITY_3D]) - std::get<float>(avgs[FEAT_ECCENTRICITY_3D]), 2.0f);
		}
		deviations[FEAT_AREA_3D] = std::sqrt(std::get<float>(deviations[FEAT_AREA_3D])/features.size());
		deviations[FEAT_MVOLUME_3D] = std::sqrt(std::get<float>(deviations[FEAT_MVOLUME_3D])/features.size());
		deviations[FEAT_BBVOLUME_3D] = std::sqrt(std::get<float>(deviations[FEAT_BBVOLUME_3D])/features.size());
		deviations[FEAT_DIAMETER_3D] = std::sqrt(std::get<float>(deviations[FEAT_DIAMETER_3D])/features.size());
		deviations[FEAT_COMPACTNESS_3D] = std::sqrt(std::get<float>(deviations[FEAT_COMPACTNESS_3D])/features.size());
		deviations[FEAT_ECCENTRICITY_3D] = std::sqrt(std::get<float>(deviations[FEAT_ECCENTRICITY_3D])/features.size());

	
		std::ofstream featsFile;
		featsFile.open("feats.csv");
		featsFile << "Path,3D_Area,3D_MVolume,3D_BBVolume,3D_Diameter,3D_Compactness,3D_Eccentricity,3D_A3,3D_D1,3D_D2,3D_D3,3D_D4\n";
		for(int i = 0; i < names.size(); i++){
			featsFile << names[i] << "," <<
				(std::get<float>(features[i][FEAT_AREA_3D]) - std::get<float>(avgs[FEAT_AREA_3D])) / std::get<float>(deviations[FEAT_AREA_3D]) << "," <<
				(std::get<float>(features[i][FEAT_MVOLUME_3D]) - std::get<float>(avgs[FEAT_MVOLUME_3D])) / std::get<float>(deviations[FEAT_MVOLUME_3D]) << "," <<
				(std::get<float>(features[i][FEAT_BBVOLUME_3D]) - std::get<float>(avgs[FEAT_BBVOLUME_3D])) / std::get<float>(deviations[FEAT_BBVOLUME_3D]) << "," <<
				(std::get<float>(features[i][FEAT_DIAMETER_3D]) - std::get<float>(avgs[FEAT_DIAMETER_3D])) / std::get<float>(deviations[FEAT_DIAMETER_3D]) << "," <<
				(std::get<float>(features[i][FEAT_COMPACTNESS_3D]) - std::get<float>(avgs[FEAT_COMPACTNESS_3D])) / std::get<float>(deviations[FEAT_COMPACTNESS_3D]) << "," << 
				(std::get<float>(features[i][FEAT_ECCENTRICITY_3D]) - std::get<float>(avgs[FEAT_ECCENTRICITY_3D])) / std::get<float>(deviations[FEAT_ECCENTRICITY_3D]) << "," <<
				(std::get<Histogram>(features[i][FEAT_A3_3D])).toString() << "," <<
				(std::get<Histogram>(features[i][FEAT_D1_3D])).toString() << "," <<
				(std::get<Histogram>(features[i][FEAT_D2_3D])).toString() << "," <<
				(std::get<Histogram>(features[i][FEAT_D3_3D])).toString() << "," <<
				(std::get<Histogram>(features[i][FEAT_D4_3D])).toString() <<
				std::endl;
		}
		featsFile.close();
		std::ofstream featsStatsFile;
		featsStatsFile.open("feats_avg.csv");
		featsStatsFile << "3D_Area_AVG,3D_Area_STD,3D_MVolume_AVG,3D_MVolume_STD,3D_BBVolume_AVG,3D_BBVolume_STD,3D_Diameter_AVG,3D_Diameter_STD,3D_Compactness_AVG,3D_Compactness_STD,3D_Eccentricity_AVG,3D_Eccentricity_STD\n";
		featsStatsFile << 
			std::get<float>(avgs[FEAT_AREA_3D]) << "," <<
			std::get<float>(deviations[FEAT_AREA_3D]) << "," <<
			std::get<float>(avgs[FEAT_MVOLUME_3D]) << "," <<
			std::get<float>(deviations[FEAT_MVOLUME_3D]) << "," <<
			std::get<float>(avgs[FEAT_BBVOLUME_3D]) << "," <<
			std::get<float>(deviations[FEAT_BBVOLUME_3D]) << "," <<
			std::get<float>(avgs[FEAT_DIAMETER_3D]) << "," <<
			std::get<float>(deviations[FEAT_DIAMETER_3D]) << "," <<
			std::get<float>(avgs[FEAT_COMPACTNESS_3D]) << "," <<
			std::get<float>(deviations[FEAT_COMPACTNESS_3D]) << "," <<
			std::get<float>(avgs[FEAT_ECCENTRICITY_3D]) << "," <<
			std::get<float>(deviations[FEAT_ECCENTRICITY_3D]) <<
			std::endl;
		featsStatsFile.close();
	
		std::filesystem::current_path(currPath);
	}
}

namespace FeatureVector {
	void getFeatureVectorClassToIndicesName(std::filesystem::path dbPath, Eigen::VectorXd& dbFeatureVector, ClassToMeshIndexNameMap& classTypeToIndicesNames, int& origDimensionality, int& numOfDataPoints) {
		std::filesystem::path featsPath = dbPath;
		featsPath /= "feats.csv";
		rapidcsv::Document feats(featsPath.string(), rapidcsv::LabelParams(0, -1));
		numOfDataPoints = feats.GetRowCount();

		std::vector<float> tempFeatureVector;
		for (int i = 0; i < numOfDataPoints; i++) {

			const auto meshPath = std::filesystem::path(feats.GetCell<std::string>("Path", i));
			const auto meshName = meshPath.filename().string();
			const auto classType = meshPath.parent_path().filename().string();
			if (classTypeToIndicesNames.find(classType) == classTypeToIndicesNames.end()) {
				std::vector<IndexNamePair> indexNamePairs;
				indexNamePairs.push_back(std::make_pair(i, meshName));
				classTypeToIndicesNames.emplace(meshPath.parent_path().filename().string(), indexNamePairs);
			}
			else {
				classTypeToIndicesNames.at(classType).push_back(std::make_pair(i, meshName));
			}

			// Compute the single-value distance (euclidean)
			tempFeatureVector.push_back(feats.GetCell<float>("3D_Area", i));
			tempFeatureVector.push_back(feats.GetCell<float>("3D_BBVolume", i));
			tempFeatureVector.push_back(feats.GetCell<float>("3D_Diameter", i));
			tempFeatureVector.push_back(feats.GetCell<float>("3D_Compactness", i));
			tempFeatureVector.push_back(feats.GetCell<float>("3D_Eccentricity", i));

			// Compute earth mover's distance
			const auto a3 = feats.GetCell<std::string>("3D_A3", i);
			const auto d1 = feats.GetCell<std::string>("3D_D1", i);
			const auto d2 = feats.GetCell<std::string>("3D_D2", i);
			const auto d3 = feats.GetCell<std::string>("3D_D3", i);
			const auto d4 = feats.GetCell<std::string>("3D_D4", i);
			const auto a3Histogram = Histogram::parseHistogram(a3);
			const auto d1Histogram = Histogram::parseHistogram(d1);
			const auto d2Histogram = Histogram::parseHistogram(d2);
			const auto d3Histogram = Histogram::parseHistogram(d3);
			const auto d4Histogram = Histogram::parseHistogram(d4);

			tempFeatureVector.insert(tempFeatureVector.end(), a3Histogram.begin(), a3Histogram.end());
			tempFeatureVector.insert(tempFeatureVector.end(), d1Histogram.begin(), d1Histogram.end());
			tempFeatureVector.insert(tempFeatureVector.end(), d2Histogram.begin(), d2Histogram.end());
			tempFeatureVector.insert(tempFeatureVector.end(), d3Histogram.begin(), d3Histogram.end());
			tempFeatureVector.insert(tempFeatureVector.end(), d4Histogram.begin(), d4Histogram.end());
			if (!origDimensionality) {
				origDimensionality = tempFeatureVector.size();
			}
		}

		std::vector<double> doubleFeatureVector(tempFeatureVector.begin(), tempFeatureVector.end());
		dbFeatureVector = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(doubleFeatureVector.data(), doubleFeatureVector.size());
	}

	void formatReducedFeatureVector(std::vector<double> flatFeatureVectors, int numOfDataPoints, Eigen::MatrixXd& reducedFeatureVectors) {
		Eigen::VectorXd normalizedData = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(flatFeatureVectors.data(), flatFeatureVectors.size());
		auto minVal = normalizedData.minCoeff();
		if (minVal < 0) {
			normalizedData.array() += abs(minVal);
		}

		normalizedData.normalize();

		typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMatrixXd;
		reducedFeatureVectors = RowMatrixXd::Map(normalizedData.data(), numOfDataPoints, 2);
	}
}

OptionsMap* OptionsMap::instance = nullptr;
