#include "shape_retriever.hpp"
#include "utils.hpp"
#include "earth_movers_distance.hpp"
#include "rapidcsv.h"

namespace Retriever {
	void retrieveSimiliarShapes(const MeshPtr& mesh, std::filesystem::path dbPath) {

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

		DescriptorMap descriptorMap = mesh->getDescriptorMap();
		descriptorMap[FEAT_DIAMETER_3D] = mesh->getConvexHull()->getDescriptor(FEAT_DIAMETER_3D);

		rapidcsv::Document feats_avg(featsAvgPath.string(), rapidcsv::LabelParams(0, -1));
		rapidcsv::Document feats(featsPath.string(), rapidcsv::LabelParams(0, -1));

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

		std::vector<float> featureVector;
		featureVector.push_back((std::get<float>(descriptorMap[FEAT_AREA_3D]) - avgArea) / stdArea);
		featureVector.push_back((std::get<float>(descriptorMap[FEAT_MVOLUME_3D]) - avgMVolume) / stdMVolume);
		featureVector.push_back((std::get<float>(descriptorMap[FEAT_BBVOLUME_3D]) - avgBBVolume) / stdBBVolume);
		featureVector.push_back((std::get<float>(descriptorMap[FEAT_DIAMETER_3D]) - avgDiameter) / stdDiameter);
		featureVector.push_back((std::get<float>(descriptorMap[FEAT_COMPACTNESS_3D]) - avgCompactness) / stdCompactness);
		featureVector.push_back((std::get<float>(descriptorMap[FEAT_ECCENTRICITY_3D]) - avgEccentricity) / stdEccentricity);


		std::cout << "Computing distances... " << std::endl;

		for (int i = 0; i < feats.GetRowCount(); i++) {

			// Compute the single-value distance (euclidean)
			std::vector<float> dbFeatureVector;
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Area", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_MVolume", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_BBVolume", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Diameter", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Compactness", i));
			dbFeatureVector.push_back(feats.GetCell<float>("3D_Eccentricity", i));

			auto singleValueDistance = vectorDistance(featureVector.begin(), featureVector.end(), dbFeatureVector.begin());

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

			const auto dba3Histogram = std::get<Histogram>(descriptorMap[FEAT_A3_3D]).getFrequency();
			const auto dbd1Histogram = std::get<Histogram>(descriptorMap[FEAT_D1_3D]).getFrequency();
			const auto dbd2Histogram = std::get<Histogram>(descriptorMap[FEAT_D2_3D]).getFrequency();
			const auto dbd3Histogram = std::get<Histogram>(descriptorMap[FEAT_D3_3D]).getFrequency();
			const auto dbd4Histogram = std::get<Histogram>(descriptorMap[FEAT_D4_3D]).getFrequency();

			std::vector<float> values = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
			const auto a3distance = std::earthMoversDistance(values, a3Histogram, values, dba3Histogram);
			const auto d1distance = std::earthMoversDistance(values, d1Histogram, values, dbd1Histogram);
			const auto d2distance = std::earthMoversDistance(values, d2Histogram, values, dbd2Histogram);
			const auto d3distance = std::earthMoversDistance(values, d3Histogram, values, dbd3Histogram);
			const auto d4distance = std::earthMoversDistance(values, d4Histogram, values, dbd4Histogram);
			similarShapes.push_back(std::make_pair(feats.GetCell<std::string>("Path", i), singleValueDistance + a3distance + d1distance + d2distance + d3distance + d4distance));
		}

		std::sort(similarShapes.begin(), similarShapes.end(), []
		(const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) {
				return a.second < b.second;
			}
		);

		mesh->setSimilarShapes(similarShapes);
	}
}