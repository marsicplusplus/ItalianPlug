#include "utils.hpp"
#include "descriptors.hpp"
#include "shape_retriever.hpp"
#include <chrono>

int main(int argc, char* args[]) {
	if (argc < 2) {
		std::cout << "USAGE:" << std::endl << args[0] << " db-path [ANN=true|false]" << std::endl;
		return 1;
	}
	std::string dbPath = args[1];
	bool useANN = false;
	const int kMax = 380;

	if(argc == 3 && strncmp(args[2], "ANN=true", strlen("ANN=true")) == 0)
		useANN = true;

	std::vector<float> mss(kMax);
	for (auto& p : std::filesystem::recursive_directory_iterator(dbPath)) {
		std::string extension = p.path().extension().string();
		std::string offExt(".off");
		std::string plyExt(".ply");
		if (extension == offExt || extension == plyExt) {
			std::cout << p << std::endl;
			MeshPtr meshPtr = std::make_shared<Mesh>(p.path().string());
			for(int i = 1; i <= kMax; i++){
				auto t1 = std::chrono::high_resolution_clock::now();
				Retriever::retrieveSimiliarShapes(meshPtr, dbPath, i, (useANN) ? Retriever::DistanceMethod::spotify_ANN : Retriever::DistanceMethod::quadratic_Weights, true);
				auto t2 = std::chrono::high_resolution_clock::now();
				auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
				mss[i] += diff.count();
			}
		}
	}
	std::string fileName = "timing_";
	fileName.append(((useANN) ? "ANN.csv" : "CUST.csv"));
	std::ofstream timingFile;
	timingFile.open(fileName);
	timingFile << "k,ms\n";
	int i = 1;
	for(auto ms : mss){
		timingFile << i++ << "," << ms / (float)kMax << std::endl;
	}
	timingFile.close();

	return 0;
}

