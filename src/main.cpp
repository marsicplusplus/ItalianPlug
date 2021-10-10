#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	bool help = false;
	bool dirStats = false;
	bool calcFeats = false;
	bool normalizeMesh = false;
	int targetVerts = 0;
	std::string meshPath;
	std::string dirPath;
	for(int i = 1; i < argc; i++){
		if(strncmp(args[i], "--help", strlen("--help")) == 0){
			help = true;
		}
		if (strncmp(args[i], "--dir-stats", strlen("--dir-stats")) == 0) {
			if (i + 1 <= argc) {
				dirStats = true;
				dirPath = args[i + 1];
			}
			else {
				std::cerr << "ERROR: it seems like you forgot to pass me the path of the database!\n" << std::flush;
				help = true;
			}
		}
		if(strncmp(args[i], "--calc-features", strlen("--calc-features")) == 0){
			if (i + 1 <= argc) {
				calcFeats = true;
				dirPath = args[i + 1];
			}
			else {
				std::cerr << "ERROR: it seems like you forgot to pass me the path of the mesh to normalize or the target n. of vertices!\n" << std::flush;
				help = true;
			}
		}
		if(strncmp(args[i], "--normalize-mesh", strlen("--normalize-mesh")) == 0){
			if (i + 2 <= argc) {
				normalizeMesh = true;
				meshPath = args[i + 1];
				targetVerts = std::strtol(args[i + 2], nullptr, 0);
			}
			else {
				std::cerr << "ERROR: it seems like you forgot to pass me the path of the Database\n" << std::flush;
				help = true;
			}
		}
	}
	if(help) {
		std::cout << "USAGE:\n";
		std::cout << args[0] << " [options]\n";
		std::cout << "Available options:\n";
		std::cout << "--help \t\t Show this message\n";
		std::cout << "--dir-stats path/to/db/ \t Recursively analyze and print the stats of every single mesh found in the passed directory\n";
		std::cout << "--calc-features path/to/db/ \t Recursively compute features for each shape in the database and store the result as a csv in the root folder;\n";
		return 0;
	}
	if(dirStats){
		Stats::getDatabaseStatistics(dirPath);
	} else if (calcFeats) {
		std::ofstream featsFile;
		std::filesystem::path fp = dirPath;
		fp /= "feats.csv";
		featsFile.open(fp);
		featsFile << "Path,3D_Area,3D_MVolume,3D_BBVolume,3D_Diameter,3D_Compactness,3D_Eccentricity,3D_A3,3D_D1,3D_D2,3D_D3,3D_D4\n";
		std::string offExt(".off");
		std::string plyExt(".ply");
		for (auto& p : std::filesystem::recursive_directory_iterator(dirPath)) {
			std::string extension = p.path().extension().string();
			if (extension == offExt || extension == plyExt) {
				std::cout << "Compute features for " << p.path().string() << std::endl;
				Mesh mesh(p.path().string());
				mesh.computeFeatures(Descriptors::descriptor_all & ~Descriptors::descriptor_diameter);
				mesh.getConvexHull()->computeFeatures(Descriptors::descriptor_diameter);
				try{
					featsFile << p.path().string() << "," <<
						std::get<float>(mesh.getDescriptor(FEAT_AREA_3D)) << "," <<
						std::get<float>(mesh.getDescriptor(FEAT_MVOLUME_3D)) << "," <<
						std::get<float>(mesh.getDescriptor(FEAT_BBVOLUME_3D)) << "," <<
						std::get<float>(mesh.getConvexHull()->getDescriptor(FEAT_DIAMETER_3D)) << "," <<
						std::get<float>(mesh.getDescriptor(FEAT_COMPACTNESS_3D)) << "," <<
						std::get<float>(mesh.getDescriptor(FEAT_ECCENTRICITY_3D)) << "," << 
						(std::get<Histogram>(mesh.getDescriptor(FEAT_A3_3D))).toString() << "," <<
						(std::get<Histogram>(mesh.getDescriptor(FEAT_D1_3D))).toString() << "," <<
						(std::get<Histogram>(mesh.getDescriptor(FEAT_D2_3D))).toString() << "," <<
						(std::get<Histogram>(mesh.getDescriptor(FEAT_D3_3D))).toString() << "," <<
						(std::get<Histogram>(mesh.getDescriptor(FEAT_D4_3D))).toString() <<
					std::endl;
				} catch(std::bad_variant_access e){
					std::cout << "Error retrieving features for " << p.path().string() << ": " << e.what();
					featsFile << p.path().string() << ",-" << std::endl;
				}
			}
		}
		featsFile.close();
	} else if (normalizeMesh) {
		Mesh mesh(meshPath);
		mesh.normalize(targetVerts);
		mesh.writeMesh();
	} else {
		Renderer rend(1024, 720, "RendererGL");
		rend.initSystems();
		rend.start();
	}
	return 0;
}
