#define IGL_HEADER_ONLY
#include "renderer.hpp"
#include "utils.hpp"

int main(int argc, char* args[]) {
	if(argc < 2){
		std::cout << "USAGE:" << std::endl << args[0] << " path-to-db" << std::endl;
		return 1;
	}
	std::string dirPath = args[1];
	Stats::getDatabaseStatistics(dirPath);
	return 0;
}
