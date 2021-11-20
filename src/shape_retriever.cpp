#include "shape_retriever.hpp"
#include "utils.hpp"
#include "earth_movers_distance.hpp"
#include "annoylib.h"
#include "kissrandom.h"
#include "rapidcsv.h"
#include <array>

namespace Retriever {

	const auto extractClass = [](std::filesystem::path filePath) {
		size_t found;
		found = filePath.string().find_last_of("/\\");
		auto classPath = filePath.string().substr(0, found);
		found = classPath.find_last_of("/\\");
		return classPath.substr(found + 1);
	};

	int buildTree(Annoy::AnnoyIndex<int, float, Annoy::Angular, Annoy::Kiss32Random, Annoy::AnnoyIndexSingleThreadedBuildPolicy> &idx, rapidcsv::Document &feats) {
		int i = 0;
		float v[DESCRIPTORS_NUM];
		for (i = 0; i < feats.GetRowCount(); i++) {
			int j = 0;
			v[j++] = (feats.GetCell<float>("3D_Area", i));
			v[j++] = (feats.GetCell<float>("3D_MVolume", i));
			v[j++] = (feats.GetCell<float>("3D_BBVolume", i));
			v[j++] = (feats.GetCell<float>("3D_Diameter", i));
			v[j++] = (feats.GetCell<float>("3D_Compactness", i));
			v[j++] = (feats.GetCell<float>("3D_Eccentricity", i));
			const auto a3Histogram = Histogram::parseHistogram(feats.GetCell<std::string>("3D_A3", i));
			for(auto val : a3Histogram)
				v[j++] = val;
			const auto d1Histogram = Histogram::parseHistogram(feats.GetCell<std::string>("3D_D1", i));
			for(auto val : d1Histogram)
				v[j++] = val;
			const auto d2Histogram = Histogram::parseHistogram(feats.GetCell<std::string>("3D_D2", i));
			for(auto val : d2Histogram)
				v[j++] = val;
			const auto d3Histogram = Histogram::parseHistogram(feats.GetCell<std::string>("3D_D3", i));
			for(auto val : d3Histogram)
				v[j++] = val;
			const auto d4Histogram = Histogram::parseHistogram(feats.GetCell<std::string>("3D_D4", i));
			for(auto val : d4Histogram)
				v[j++] = val;
			idx.add_item(i, v);
		}
		return i;
	}

	void retrieveSimiliarShapesANN(const MeshPtr& mesh, std::filesystem::path dbPath, int shapes, bool includeSelf) {

		std::filesystem::path featsAvgPath = dbPath;
		featsAvgPath /= "feats_avg.csv";
		if (!std::filesystem::exists(featsAvgPath)) {
			std::cout << "Could not find " << featsAvgPath << ".\nRun FeaturesExtractor on the mesh DB to generate the average feature file first" << std::endl;
			return;
		}
		rapidcsv::Document feats((dbPath / "feats.csv").string(), rapidcsv::LabelParams(0, -1));

		DescriptorMap descriptorMap;
		// Initialize the query mesh histograms before searching the db for the q mesh
		std::vector<float> qa3Histogram;
		std::vector<float> qd1Histogram;
		std::vector<float> qd2Histogram;
		std::vector<float> qd3Histogram;
		std::vector<float> qd4Histogram;

		auto idx = Annoy::AnnoyIndex<int, float, Annoy::Angular, Annoy::Kiss32Random, Annoy::AnnoyIndexSingleThreadedBuildPolicy>(DESCRIPTORS_NUM);
		float v[DESCRIPTORS_NUM];
		int i = 0;
		auto row_count = feats.GetRowCount();
		rapidcsv::Document feats_avg(featsAvgPath.string(), rapidcsv::LabelParams(0, -1));

		int meshIndex = 0;
		bool meshInDB = false;
		auto meshPath = mesh->getPath();
		auto className = extractClass(meshPath);
		if (meshPath.string().find(dbPath.string()) != std::string::npos) {
			auto meshFilename = className + "/" + meshPath.filename().string();
			for (int i = 0; i < feats.GetRowCount(); i++) {
				if (feats.GetCell<std::string>("Path", i).find(meshFilename) != std::string::npos) {
					meshIndex = i;
					meshInDB = true;
					descriptorMap[FEAT_AREA_3D] = feats.GetCell<float>("3D_Area", i);
					descriptorMap[FEAT_MVOLUME_3D] = feats.GetCell<float>("3D_MVolume", i);
					descriptorMap[FEAT_BBVOLUME_3D] = feats.GetCell<float>("3D_BBVolume", i);
					descriptorMap[FEAT_DIAMETER_3D] = feats.GetCell<float>("3D_Diameter", i);
					descriptorMap[FEAT_COMPACTNESS_3D] = feats.GetCell<float>("3D_Compactness", i);
					descriptorMap[FEAT_ECCENTRICITY_3D] = feats.GetCell<float>("3D_Eccentricity", i);

					const auto a3 = feats.GetCell<std::string>("3D_A3", i);
					const auto d1 = feats.GetCell<std::string>("3D_D1", i);
					const auto d2 = feats.GetCell<std::string>("3D_D2", i);
					const auto d3 = feats.GetCell<std::string>("3D_D3", i);
					const auto d4 = feats.GetCell<std::string>("3D_D4", i);
					const auto dba3Histogram = Histogram::parseHistogram(a3);
					const auto dbd1Histogram = Histogram::parseHistogram(d1);
					const auto dbd2Histogram = Histogram::parseHistogram(d2);
					const auto dbd3Histogram = Histogram::parseHistogram(d3);
					const auto dbd4Histogram = Histogram::parseHistogram(d4);

					qa3Histogram = dba3Histogram;
					qd1Histogram = dbd1Histogram;
					qd2Histogram = dbd2Histogram;
					qd3Histogram = dbd3Histogram;
					qd4Histogram = dbd4Histogram;
					break;
				}
			}
		}

		if (!meshInDB) {
			i = buildTree(idx, feats);

			mesh->computeFeatures(Descriptors::descriptor_all & ~Descriptors::descriptor_diameter);
			mesh->getConvexHull()->computeFeatures(Descriptors::descriptor_diameter);
			descriptorMap = mesh->getDescriptorMap();
			descriptorMap[FEAT_DIAMETER_3D] = mesh->getConvexHull()->getDescriptor(FEAT_DIAMETER_3D);
			qa3Histogram = std::get<Histogram>(descriptorMap[FEAT_A3_3D]).getFrequency();
			qd1Histogram = std::get<Histogram>(descriptorMap[FEAT_D1_3D]).getFrequency();
			qd2Histogram = std::get<Histogram>(descriptorMap[FEAT_D2_3D]).getFrequency();
			qd3Histogram = std::get<Histogram>(descriptorMap[FEAT_D3_3D]).getFrequency();
			qd4Histogram = std::get<Histogram>(descriptorMap[FEAT_D4_3D]).getFrequency();

			const auto avgArea = feats_avg.GetColumn<float>("3D_Area_AVG")[0];
			const auto stdArea = feats_avg.GetColumn<float>("3D_Area_STD")[0];
			const auto avgMVolume = feats_avg.GetColumn<float>("3D_MVolume_AVG")[0];
			const auto stdMVolume = feats_avg.GetColumn<float>("3D_MVolume_STD")[0];
			const auto avgBBVolume = feats_avg.GetColumn<float>("3D_BBVolume_AVG")[0];
			const auto stdBBVolume = feats_avg.GetColumn<float>("3D_BBVolume_STD")[0];
			const auto avgDiameter = feats_avg.GetColumn<float>("3D_Diameter_AVG")[0];
			const auto stdDiameter = feats_avg.GetColumn<float>("3D_Diameter_STD")[0];
			const auto avgCompactness = feats_avg.GetColumn<float>("3D_Compactness_AVG")[0];
			const auto stdCompactness = feats_avg.GetColumn<float>("3D_Compactness_STD")[0];
			const auto avgEccentricity = feats_avg.GetColumn<float>("3D_Eccentricity_AVG")[0];
			const auto stdEccentricity = feats_avg.GetColumn<float>("3D_Eccentricity_STD")[0];

			int j = 0;
			v[j++] = ((std::get<float>(descriptorMap[FEAT_AREA_3D]) - avgArea) / stdArea);
			v[j++] = ((std::get<float>(descriptorMap[FEAT_MVOLUME_3D]) - avgMVolume) / stdMVolume);
			v[j++] = ((std::get<float>(descriptorMap[FEAT_BBVOLUME_3D]) - avgBBVolume) / stdBBVolume);
			v[j++] = ((std::get<float>(descriptorMap[FEAT_DIAMETER_3D]) - avgDiameter) / stdDiameter);
			v[j++] = ((std::get<float>(descriptorMap[FEAT_COMPACTNESS_3D]) - avgCompactness) / stdCompactness);
			v[j++] =  ((std::get<float>(descriptorMap[FEAT_ECCENTRICITY_3D]) - avgEccentricity) / stdEccentricity);

			for (auto val : qa3Histogram)
				v[j++] = val;
			for (auto val : qd1Histogram)
				v[j++] = (val);
			for (auto val : qd2Histogram)
				v[j++] = (val);
			for (auto val : qd3Histogram)
				v[j++] = (val);
			for (auto val : qd4Histogram)
				v[j++] = (val);
			idx.add_item(i, v);
			meshIndex = i;
			idx.build(DESCRIPTORS_NUM * 2);
		} else {
			if(std::filesystem::exists(dbPath/"ann_tree.ann")){
				idx.load((dbPath/"ann_tree.ann").c_str());
			} else {
				i = buildTree(idx, feats);
				idx.build(DESCRIPTORS_NUM * 2);
				idx.save((dbPath/"ann_tree.ann").c_str());
			}
		}

		std::vector<int> result;
		result.reserve(shapes);
		std::vector<float> distances;
		distances.reserve(shapes);

		// If the mesh is outside the DB we want to return shapes + 1 as the first entry is itself (but it's not stored in the DB so we can't access it in the feats file)
		// If the mesh is inside the DB and we want to return itself, then we want to just use shapes
		int startIndex = includeSelf && meshInDB ? 0 : 1;
		int numToRetrieve = includeSelf && meshInDB ? shapes : shapes + 1;

		idx.get_nns_by_item(meshIndex, numToRetrieve, -1, &result, &distances);
		idx.unload();
		std::vector<std::pair<std::string, float>> similarShapes;
		for(i = startIndex; i < result.size(); i++){
			similarShapes.push_back(std::make_pair(feats.GetCell<std::string>("Path", result[i]), distances[i]));
		}
		mesh->setSimilarShapes(similarShapes);
	}

	void retrieveSimiliarShapesCUST(const MeshPtr& mesh, std::filesystem::path dbPath, bool includeSelf, std::array<float, 6> scalarWeights, std::array<float, 6> functionWeights, bool squareDistance, bool useEMD, bool useSqrt) {

		std::vector<std::pair<std::string, float>> similarShapes;

		std::filesystem::path featsPath = dbPath;
		featsPath /= "feats.csv";
		if (!std::filesystem::exists(featsPath)) {
			std::cout << "Could not find " << featsPath << ".\nRun FeaturesExtractor on the mesh DB to generate the feature file first" << std::endl;
			return;
		}

		std::filesystem::path featsAvgPath = dbPath;
		featsAvgPath /= "feats_avg.csv";
		if (!std::filesystem::exists(featsAvgPath)) {
			std::cout << "Could not find " << featsAvgPath << ".\nRun FeaturesExtractor on the mesh DB to generate the average feature file first" << std::endl;
			return;
		}

		rapidcsv::Document feats_avg(featsAvgPath.string(), rapidcsv::LabelParams(0, -1));
		rapidcsv::Document feats(featsPath.string(), rapidcsv::LabelParams(0, -1));

		DescriptorMap descriptorMap;
		std::vector<float> qa3Histogram;
		std::vector<float> qd1Histogram;
		std::vector<float> qd2Histogram;
		std::vector<float> qd3Histogram;
		std::vector<float> qd4Histogram;
		std::vector<float> featureVector;

		bool meshInDB = false;
		auto meshPath = mesh->getPath();
		auto className = extractClass(meshPath);
		if (meshPath.string().find(dbPath.string()) != std::string::npos) {
			auto meshFilename = className + "/" + meshPath.filename().string();
			for (int i = 0; i < feats.GetRowCount(); i++) {
				if (feats.GetCell<std::string>("Path", i).find(meshFilename) != std::string::npos) {
					meshInDB = true;
					// If the mesh is in the database the values have already been normalized
					featureVector.push_back(feats.GetCell<float>("3D_Area", i));
					featureVector.push_back(feats.GetCell<float>("3D_MVolume", i));
					featureVector.push_back(feats.GetCell<float>("3D_BBVolume", i));
					featureVector.push_back(feats.GetCell<float>("3D_Diameter", i));
					featureVector.push_back(feats.GetCell<float>("3D_Compactness", i));
					featureVector.push_back(feats.GetCell<float>("3D_Eccentricity", i));

					const auto a3 = feats.GetCell<std::string>("3D_A3", i);
					const auto d1 = feats.GetCell<std::string>("3D_D1", i);
					const auto d2 = feats.GetCell<std::string>("3D_D2", i);
					const auto d3 = feats.GetCell<std::string>("3D_D3", i);
					const auto d4 = feats.GetCell<std::string>("3D_D4", i);

					qa3Histogram = Histogram::parseHistogram(a3);
					qd1Histogram = Histogram::parseHistogram(d1);
					qd2Histogram = Histogram::parseHistogram(d2);
					qd3Histogram = Histogram::parseHistogram(d3);
					qd4Histogram = Histogram::parseHistogram(d4);

					break;
				}
			}
		}

		if (!meshInDB) {
			mesh->computeFeatures(Descriptors::descriptor_all & ~Descriptors::descriptor_diameter);
			mesh->getConvexHull()->computeFeatures(Descriptors::descriptor_diameter);
			descriptorMap = mesh->getDescriptorMap();
			descriptorMap[FEAT_DIAMETER_3D] = mesh->getConvexHull()->getDescriptor(FEAT_DIAMETER_3D);
			qa3Histogram = std::get<Histogram>(descriptorMap[FEAT_A3_3D]).getFrequency();
			qd1Histogram = std::get<Histogram>(descriptorMap[FEAT_D1_3D]).getFrequency();
			qd2Histogram = std::get<Histogram>(descriptorMap[FEAT_D2_3D]).getFrequency();
			qd3Histogram = std::get<Histogram>(descriptorMap[FEAT_D3_3D]).getFrequency();
			qd4Histogram = std::get<Histogram>(descriptorMap[FEAT_D4_3D]).getFrequency();

			const auto avgArea = feats_avg.GetColumn<float>("3D_Area_AVG")[0];
			const auto stdArea = feats_avg.GetColumn<float>("3D_Area_STD")[0];
			const auto avgMVolume = feats_avg.GetColumn<float>("3D_MVolume_AVG")[0];
			const auto stdMVolume = feats_avg.GetColumn<float>("3D_MVolume_STD")[0];
			const auto avgBBVolume = feats_avg.GetColumn<float>("3D_BBVolume_AVG")[0];
			const auto stdBBVolume = feats_avg.GetColumn<float>("3D_BBVolume_STD")[0];
			const auto avgDiameter = feats_avg.GetColumn<float>("3D_Diameter_AVG")[0];
			const auto stdDiameter = feats_avg.GetColumn<float>("3D_Diameter_STD")[0];
			const auto avgCompactness = feats_avg.GetColumn<float>("3D_Compactness_AVG")[0];
			const auto stdCompactness = feats_avg.GetColumn<float>("3D_Compactness_STD")[0];
			const auto avgEccentricity = feats_avg.GetColumn<float>("3D_Eccentricity_AVG")[0];
			const auto stdEccentricity = feats_avg.GetColumn<float>("3D_Eccentricity_STD")[0];

			featureVector.push_back((std::get<float>(descriptorMap[FEAT_AREA_3D]) - avgArea) / stdArea);
			featureVector.push_back((std::get<float>(descriptorMap[FEAT_MVOLUME_3D]) - avgMVolume) / stdMVolume);
			featureVector.push_back((std::get<float>(descriptorMap[FEAT_BBVOLUME_3D]) - avgBBVolume) / stdBBVolume);
			featureVector.push_back((std::get<float>(descriptorMap[FEAT_DIAMETER_3D]) - avgDiameter) / stdDiameter);
			featureVector.push_back((std::get<float>(descriptorMap[FEAT_COMPACTNESS_3D]) - avgCompactness) / stdCompactness);
			featureVector.push_back((std::get<float>(descriptorMap[FEAT_ECCENTRICITY_3D]) - avgEccentricity) / stdEccentricity);
		}

		for (int i = 0; i < feats.GetRowCount(); i++) {

			// Compute the single-value distance (euclidean)
			std::vector<float> dbFeatureVector;
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Area", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_MVolume", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_BBVolume", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Diameter", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Compactness", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Eccentricity", i));

			auto singleValueDistance = vectorDistance(featureVector.begin(), featureVector.end(), dbFeatureVector.begin(), scalarWeights.begin(), squareDistance, useSqrt);

			// Compute earth mover's distance
			const auto a3 = feats.GetCell<std::string>("3D_A3", i);
			const auto d1 = feats.GetCell<std::string>("3D_D1", i);
			const auto d2 = feats.GetCell<std::string>("3D_D2", i);
			const auto d3 = feats.GetCell<std::string>("3D_D3", i);
			const auto d4 = feats.GetCell<std::string>("3D_D4", i);
			const auto dba3Histogram = Histogram::parseHistogram(a3);
			const auto dbd1Histogram = Histogram::parseHistogram(d1);
			const auto dbd2Histogram = Histogram::parseHistogram(d2);
			const auto dbd3Histogram = Histogram::parseHistogram(d3);
			const auto dbd4Histogram = Histogram::parseHistogram(d4);

			auto a3distance = 0.0f;
			auto d1distance = 0.0f;
			auto d2distance = 0.0f;
			auto d3distance = 0.0f;
			auto d4distance = 0.0f;

			if (useEMD) {
				std::vector<float> values = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };

				a3distance = std::earthMoversDistance(values, qa3Histogram, values, dba3Histogram);
				d1distance = std::earthMoversDistance(values, qd1Histogram, values, dbd1Histogram);
				d2distance = std::earthMoversDistance(values, qd2Histogram, values, dbd2Histogram);
				d3distance = std::earthMoversDistance(values, qd3Histogram, values, dbd3Histogram);
				d4distance = std::earthMoversDistance(values, qd4Histogram, values, dbd4Histogram);
			} else {
				a3distance = vectorDistance(qa3Histogram.begin(), qa3Histogram.end(), dba3Histogram.begin(), scalarWeights.begin(), squareDistance, useSqrt);
				d1distance = vectorDistance(qd1Histogram.begin(), qd1Histogram.end(), dbd1Histogram.begin(), scalarWeights.begin(), squareDistance, useSqrt);
				d2distance = vectorDistance(qd2Histogram.begin(), qd2Histogram.end(), dbd2Histogram.begin(), scalarWeights.begin(), squareDistance, useSqrt);
				d3distance = vectorDistance(qd3Histogram.begin(), qd3Histogram.end(), dbd3Histogram.begin(), scalarWeights.begin(), squareDistance, useSqrt);
				d4distance = vectorDistance(qd4Histogram.begin(), qd4Histogram.end(), dbd4Histogram.begin(), scalarWeights.begin(), squareDistance, useSqrt);
			}


			singleValueDistance = singleValueDistance * functionWeights[0];
			a3distance = a3distance * functionWeights[1];
			d1distance = d1distance * functionWeights[2];
			d2distance = d2distance * functionWeights[3];
			d3distance = d3distance * functionWeights[4];
			d4distance = d4distance * functionWeights[5];

			similarShapes.push_back(std::make_pair(feats.GetCell<std::string>("Path", i), singleValueDistance + a3distance + d1distance + d2distance + d3distance + d4distance));
		}

		std::sort(similarShapes.begin(), similarShapes.end(), []
		(const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) {
				return a.second < b.second;
			}
		);

		if (!includeSelf) {
			similarShapes.erase(similarShapes.begin());
		}

		mesh->setSimilarShapes(similarShapes);
	}

	void retrieveSimiliarShapes(const MeshPtr& mesh, std::filesystem::path dbPath, int shapes, DistanceMethod method, bool includeSelf) {
		std::array<float, 6> functionWeights;
		std::array<float, 6> scalarWeights;
		bool useSqrt = false;
		bool squareDistance = true;
		bool useEMD = true;
		switch (method) {
		case DistanceMethod::eucliden_NoWeights:
			useSqrt = true;
			scalarWeights = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
			functionWeights = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
			retrieveSimiliarShapesCUST(mesh, dbPath, includeSelf, scalarWeights, functionWeights, squareDistance, useEMD, useSqrt);
			break;
		case DistanceMethod::quadratic_Weights:
			scalarWeights = { 3.0f / 12.0f, 3.0f / 12.0f , 0.5f / 12.0f, 0.5f / 12.0f, 3.0f / 12.0f, 2.0f / 12.0f };
			functionWeights = { .8f / 12.0f, 1.9f / 12.0f, 3.6f / 12.0f, 1.9f / 12.0f, 1.9f / 12.0f, 1.9f / 12.0f };
			retrieveSimiliarShapesCUST(mesh, dbPath, includeSelf, scalarWeights, functionWeights, squareDistance, useEMD, useSqrt);
			break;
		case DistanceMethod::flat_NoWeights:
			squareDistance = false;
			useEMD = false;
			scalarWeights = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
			functionWeights = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
			retrieveSimiliarShapesCUST(mesh, dbPath, includeSelf, scalarWeights, functionWeights, squareDistance, useEMD, useSqrt);
			break;
		case DistanceMethod::emd_NoWeights:
			squareDistance = false;
			scalarWeights = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
			functionWeights = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
			retrieveSimiliarShapesCUST(mesh, dbPath, includeSelf, scalarWeights, functionWeights, squareDistance, useEMD, useSqrt);
			break;
		case DistanceMethod::spotify_ANN:
			retrieveSimiliarShapesANN(mesh, dbPath, shapes, includeSelf);
			break;
		}
	}
}
