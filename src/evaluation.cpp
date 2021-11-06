#include <iostream>
#include <filesystem>
#include "renderer.hpp"
#include "shape_retriever.hpp"

int main(int argc, char* args[]) {
	if(argc < 2) {
		printf("USAGE:\n %s db-path\n", args[0]);
		return 1;
	}
	std::filesystem::path dbPath = args[1];

	//std::filesystem::path dbPath = "D:\\Projects\\GMT\\MultimediaRetrievalDatasets\\labeledDb\\NormalizedDB";

	const int meshesPerClass = 20;
	const int totalMeshes = 380;
	const int kMax = meshesPerClass;
	const bool useKNN = false;

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
	float dbTP = 0.0f;
	float dbFP = 0.0f;
	float dbTN = 0.0f;
	float dbFN = 0.0f;

	std::vector<std::pair<float, float>> rocPair(kMax);

	std::string evalFilename = "eval";
	evalFilename += useKNN ? "_ANN_" : "_CUST_";
	evalFilename += std::to_string(kMax) + ".csv";
	std::ofstream evalFile;
	evalFile.open(evalFilename);
	evalFile << "Class,MAP,MAR,Accuracy,F1,Specificity\n";

	std::cout << "class,MAP,MAR,Accuracy,F1,Specificity" << std::endl;
	for (auto& dir : std::filesystem::recursive_directory_iterator(dbPath)) {
		if(std::filesystem::is_directory(dir)){
			float classMAP = 0.0f;
			float classAccuracy = 0.0f;
			float classMAR = 0.0f;
			float classF1 = 0.0f;
			float classSpecificity = 0.0f;
			float MAR = 0.0f;
			float MAP = 0.0f;
			
			for (auto& p : std::filesystem::recursive_directory_iterator(dir)) {
				if(isMesh(p)){
					int TP = 0, FP = 0, TN = 0, FN = 0;
					std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(p.path().string());
					if (useKNN) {
						Retriever::retrieveSimiliarShapesKNN(mesh, dbPath, kMax);
					} else {
						Retriever::retrieveSimiliarShapes(mesh, dbPath);
					}
					const auto similarShapes = mesh->getSimilarShapes();

					if (!similarShapes.empty()) {
						std::string meshClass = extractClass(p);
						float precision = 0.0f; 	// TP / s
						float recall = 0.0f; 	// TP / c
						float specificity = 0.0f;
						/* MAP, MAR, Recall, Specificity*/
						for(auto i = 0; i < kMax; ++i){
							if(extractClass(similarShapes[i].first) == meshClass) ++TP;
							else ++FP;
							precision = (TP / (float)(i + 1));
							recall = (TP / (float)meshesPerClass);
							MAP += precision;
							MAR += recall;
							TN = totalMeshes - meshesPerClass - FP;
							specificity = TN / float(totalMeshes - meshesPerClass);
							rocPair[i].first += specificity;
							rocPair[i].second += recall;
						}

						MAP /= (float)kMax;
						MAR /= (float)kMax;

						classMAP += MAP;
						classMAR += MAR;

						dbMAP += MAP;
						dbMAR += MAR;

						/* Accuracy, F1, Specificity for k=kMax (20)*/
						FN = meshesPerClass - TP;
						float accuracy = ((TP + TN) / (float) totalMeshes);
						float F1 = (precision + recall == 0) ? 0 : 2 * (float)((precision * recall) / (float)(precision + recall));

						classAccuracy += accuracy;
						classSpecificity += specificity;
						classF1 += F1;

						dbAccuracy += accuracy;
						dbSpecificity += specificity;
						dbF1 += F1;
					}
				}
			}
			classMAP /= (float)meshesPerClass;
			classAccuracy /= (float)meshesPerClass;
			classMAR /= (float)meshesPerClass;
			classF1 /= (float)meshesPerClass;
			std::cout << dir << "," << classMAP << "," << classMAR << "," << classAccuracy << "," << classF1 << "," << classSpecificity << std::endl;
			evalFile << dir << "," << classMAP << "," << classMAR << "," << classAccuracy << "," << classF1 << "," << classSpecificity << std::endl;
		}
	}

	dbAccuracy /= (float)totalMeshes;
	dbMAP /= (float)totalMeshes;
	dbMAR /= (float)totalMeshes;
	dbF1 /= (float)totalMeshes;
	dbSpecificity /= (float)totalMeshes;

	std::cout << std::endl;
	std::cout << "dbAccuracy,dbMAP,dbMAR,dbF1,dbSpecificity" <<std::endl << dbAccuracy << "," << dbMAP << "," << dbMAR << "," << dbF1 << ',' << dbSpecificity << std::endl;
	evalFile << "Whole DB" << "," << dbMAP << "," << dbMAR << "," << dbAccuracy << "," << dbF1 << ',' << dbSpecificity << std::endl;
	evalFile.close();

	std::string rocFilename = "roc";
	rocFilename += useKNN ? "_ANN_" : "_CUST_";
	rocFilename += std::to_string(kMax) + ".csv";
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
