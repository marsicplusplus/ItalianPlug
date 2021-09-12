#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "glm/vec3.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
};

namespace Loader {
	bool loadOFF(std::string fileName, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices);
	bool loadPLY(std::string fileName, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices);
};

#endif
