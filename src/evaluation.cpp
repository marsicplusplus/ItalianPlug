#include <iostream>
#include <filesystem>
#include "renderer.hpp"
#include "shape_retriever.hpp"

std::string generateFilename(int kMax, Retriever::DistanceMethod distanceMethod, bool isROC = false) {
	std::string filename = "";
	filename += isROC ? "ROC_" : "EVAL_";
	filename += std::to_string(kMax) + "_";
	switch (distanceMethod) {
	case Retriever::DistanceMethod::eucliden_NoWeights:
		filename += "Euclidean_NoWeights";
		break;
	case Retriever::DistanceMethod::quadratic_Weights:
		filename += "Quadratic_Weights";
		break;
	case Retriever::DistanceMethod::flat_NoWeights:
		filename += "Flat_NoWeights";
		break;
	case Retriever::DistanceMethod::spotify_ANN:
		filename += "Spotify_ANN";
		break;
	}
	filename += ".csv";
	return filename;
}

int main(int argc, char* args[]) {
	if(argc < 2) {
		printf("USAGE:\n %s db-path [method= 0 (eucliden_NoWeights) | 1 (quadratic_Weights) | 2 (flat_NoWeights) | 3 (spotify_ANN)]\n", args[0]);
		return 1;
	}

	std::filesystem::path dbPath = args[1];

	const int meshesPerClass = 20;
	const int numClasses = 19;
	const int totalMeshes = 380;
	const int kMax = 20;
	Retriever::DistanceMethod distanceMethod = static_cast<Retriever::DistanceMethod>(atoi(args[2]));

	const auto extractClass = [](std::filesystem::path filePath) {
		size_t found;
		found = filePath.string().find_last_of("/\\");
		auto classPath = filePath.string().substr(0, found);
		found = classPath.find_last_of("/\\");
		return classPath.substr(found + 1);
	};

	const auto isMesh = [](std::filesystem::path mesh) {
		const std::string extension = mesh.extension().string();
		return extension == ".off" || extension == ".py";
	};

	float dbMAP = 0.0f;
	float dbF1 = 0.0f;
	float dbMAR = 0.0f;
	float dbAccuracy = 0.0f;
	float dbSpecificity = 0.0f;
	float dbLastRank = 0.0f;
	float dbTP = 0.0f;
	float dbFP = 0.0f;
	float dbTN = 0.0f;
	float dbFN = 0.0f;

	bool calculateKthTier = kMax >= meshesPerClass * 5;
	std::vector<std::vector<float>> kthTiers;

	std::vector<std::pair<float, float>> rocPair(kMax);


	std::string evalFilename = generateFilename(kMax, distanceMethod, false);
	std::ofstream evalFile;
	evalFile.open(evalFilename);
	evalFile << "Class,MAP,MAR,Accuracy,F1,Specificity,LastRank,1stTier,2ndTier,3rdTier,4thTier,5thTier\n";

	std::cout << "class,MAP,MAR,Accuracy,F1,Specificity,LastRank,1stTier,2ndTier,3rdTier,4thTier,5thTier" << std::endl;
	for (auto& dir : std::filesystem::recursive_directory_iterator(dbPath)) {
		if (std::filesystem::is_directory(dir)) {
			float classMAP = 0.0f;
			float classAccuracy = 0.0f;
			float classMAR = 0.0f;
			float classF1 = 0.0f;
			float classSpecificity = 0.0f;
			float classLastRank = 0.0f;
			std::vector<float> classKthTier(totalMeshes / meshesPerClass);


			for (auto& p : std::filesystem::recursive_directory_iterator(dir)) {
				if (isMesh(p)) {
					int TP = 0, FP = 0, TN = 0, FN = 0;
					float shapeMAP = 0.0f;
					float shapeMAR = 0.0f;
					int lastRank = 0;
					bool lastRankFound = false;
					std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(p.path().string());
					Retriever::retrieveSimiliarShapes(mesh, dbPath, kMax, distanceMethod, true);
					const auto similarShapes = mesh->getSimilarShapes();

					if (!similarShapes.empty()) {
						std::string meshClass = extractClass(p);
						float precision = 0.0f; 	// TP / s
						float recall = 0.0f; 	// TP / c
						float specificity = 0.0f;
						/* MAP, MAR, Recall, Specificity*/
						for (auto i = 0; i < kMax; ++i) {
							if (extractClass(similarShapes[i].first) == meshClass) {
								if (calculateKthTier) {
									auto tierIndex = i / meshesPerClass;
									classKthTier[tierIndex]++;
								}
								++TP;
							}
							else {
								++FP;
								if (!lastRankFound) {
									lastRank = i - 1;
								}
								lastRankFound = true;
							}
							precision = (TP / (float)(i + 1));
							recall = (TP / (float)meshesPerClass);
							shapeMAP += precision;
							shapeMAR += recall;
							TN = totalMeshes - meshesPerClass - FP;
							specificity = TN / float(totalMeshes - meshesPerClass);
							rocPair[i].first += specificity;
							rocPair[i].second += recall;
						}

						shapeMAP /= (float)kMax;
						shapeMAR /= (float)kMax;

						classMAP += shapeMAP;
						classMAR += shapeMAR;

						dbMAP += shapeMAP;
						dbMAR += shapeMAR;

						/* Accuracy, F1, Specificity for k=kMax (20)*/
						FN = meshesPerClass - TP;
						float accuracy = ((TP + TN) / (float)totalMeshes);
						float F1 = (precision + recall == 0) ? 0 : 2 * (float)((precision * recall) / (float)(precision + recall));

						classAccuracy += accuracy;
						classSpecificity += specificity;
						classF1 += F1;
						classLastRank += lastRank;

						dbAccuracy += accuracy;
						dbSpecificity += specificity;
						dbF1 += F1;
						dbLastRank += lastRank;
					}
				}
			}
			std::transform(classKthTier.begin(), classKthTier.end(), classKthTier.begin(), [meshesPerClass](float& val) { return val / (meshesPerClass * meshesPerClass); });
			kthTiers.push_back(classKthTier);
			classMAP /= (float)meshesPerClass;
			classAccuracy /= (float)meshesPerClass;
			classMAR /= (float)meshesPerClass;
			classF1 /= (float)meshesPerClass;
			classSpecificity /= (float)meshesPerClass;
			classLastRank /= (float)meshesPerClass;
			std::cout << dir << "," << classMAP << "," << classMAR << "," << classAccuracy << "," << classF1 << "," << classSpecificity << "," << classLastRank << std::endl;
			evalFile << 
				dir << "," << 
				classMAP << "," << 
				classMAR << "," << 
				classAccuracy << "," << 
				classF1 << "," << 
				classSpecificity << "," << 
				classLastRank << "," <<
				classKthTier[0] << "," <<
				classKthTier[1] << "," <<
				classKthTier[2] << "," <<
				classKthTier[3] << "," <<
				classKthTier[4] << std::endl;
		}
	}

	std::vector<float> dbKthTier(5);
	for (const auto& classTier : kthTiers) {
		for (int i = 0; i < 5; i++) {
			dbKthTier[i] += classTier[i];
		}
	}


	dbAccuracy /= (float)totalMeshes;
	dbMAP /= (float)totalMeshes;
	dbMAR /= (float)totalMeshes;
	dbF1 /= (float)totalMeshes;
	dbSpecificity /= (float)totalMeshes;
	dbLastRank /= (float)totalMeshes;
	std::transform(dbKthTier.begin(), dbKthTier.end(), dbKthTier.begin(), [numClasses](float& val) { return val / numClasses; });

	std::cout << std::endl;
	std::cout << "dbAccuracy,dbMAP,dbMAR,dbF1,dbSpecificity,dbLastRank" << std::endl << dbAccuracy << "," << dbMAP << "," << dbMAR << "," << dbF1 << ',' << dbSpecificity << ',' << dbLastRank << std::endl;
	evalFile << "Whole DB" << "," <<
		dbMAP << "," << 
		dbMAR << "," << 
		dbAccuracy << "," << 
		dbF1 << ',' << 
		dbSpecificity << ',' << 
		dbLastRank << ',' <<
		dbKthTier[0] << "," <<
		dbKthTier[1] << "," <<
		dbKthTier[2] << "," <<
		dbKthTier[3] << "," <<
		dbKthTier[4] << std::endl;

	evalFile.close();

	std::string rocFilename = generateFilename(kMax, distanceMethod, true);
	std::ofstream rocFile;
	rocFile.open(rocFilename);
	rocFile << "Specificity,Recall\n";

	for (auto& pair : rocPair) {
		pair.first /= (float)totalMeshes;
		pair.second /= (float)totalMeshes;
		rocFile << pair.first << "," << pair.second << std::endl;
	}
	rocFile.close();
}

