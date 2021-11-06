#include <iostream>
#include <filesystem>
#include "renderer.hpp"
#include "shape_retriever.hpp"

int main(int argc, char* args[]){
	if(argc < 2) {
		printf("USAGE:\n %s db-path\n", args[0]);
		return 1;
	}
	std::filesystem::path dbPath = args[1];

	const int meshesPerClass = 20;
	const int totalMeshes = 380;
	const int kMax= 20;

	const auto extractClass = [](std::filesystem::path mesh){
		int b = mesh.string().find_last_of("/");
		std::string tmp = mesh.string().substr(0, b);
		int a = tmp.find_last_of("/");
		return mesh.string().substr(a+1, b-(a+1));
	};

	const auto isMesh = [](std::filesystem::path mesh){
		const std::string extension = mesh.extension().string();
		return extension == ".off" || extension == ".py";
	};

	float dbMAP = 0.0f;
	float dbRecall = 0.0f;
	float dbAccuracy = 0.0f;
	float dbTP = 0.0f;
	float dbFP = 0.0f;
	float dbTN = 0.0f;
	float dbFN = 0.0f;

	std::cout << "class,precision,accuracy" << std::endl;
	for (auto& dir : std::filesystem::recursive_directory_iterator(dbPath)) {
		if(std::filesystem::is_directory(dir)){
			float classMAP = 0.0f;
			float classAccuracy = 0.0f;
			
			for (auto& p : std::filesystem::recursive_directory_iterator(dir)) {
				if(isMesh(p)){
					int TP = 0, FP = 0, TN = 0, FN = 0;
					std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(p.path().string());
					Retriever::retrieveSimiliarShapes(mesh, dbPath);
					const auto similarShapes = mesh->getSimilarShapes();

					if (!similarShapes.empty()) {
						std::string meshClass = extractClass(p);
						float precision = 0.0f; 	// TP / s
						//float recall = 0.0f;		// TP / c

						/* MAP */
						for(auto j = 1; j <= kMax; ++j){
							TP = FP = TN = FN = 0;
							for(auto i = 0; i < j; ++i){
								if(extractClass(similarShapes[i].first) == meshClass) ++TP;
								else ++FP;
							}
							FN = meshesPerClass - TP;
							TN = totalMeshes - meshesPerClass - TP;
							precision += (TP / (float) j);
							//recall += localTP / (float) meshesPerClass;
						}
						precision /= kMax;
						classMAP += precision;
						//recall /= kMax;
						float accuracy = ((TP + TN) / (float) totalMeshes);
						classAccuracy += accuracy;
						dbAccuracy +=  accuracy;
						dbMAP += precision;
					}
				}
			}
			classMAP /= meshesPerClass;
			classAccuracy /= meshesPerClass;
			std::cout << dir << "," << classMAP << "," << classAccuracy << std::endl;
		}
	}
	std::cout << std::endl;
	std::cout << "dbAccuracy, dbMAP"<<std::endl<<dbAccuracy/(float)totalMeshes<<","<<dbMAP/(float)totalMeshes<<std::endl;
}
