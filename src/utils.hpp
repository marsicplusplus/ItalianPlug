#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "glm/vec3.hpp"
#include <unordered_map>
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

enum Options{
	DRAW_EDGES,
};

class OptionsMap{
	public:
		static OptionsMap *Instance(){
			if(instance == nullptr){
				instance = new OptionsMap();
			}
			return instance;
		}

		inline void destroy(){
			delete instance;
		}

		inline void setOption(Options opt, bool v){
			opts[opt] = v;
		}

		inline bool getOption(Options opt){
			return opts[opt];
		}

	private:
		std::unordered_map<Options, bool> opts;

		OptionsMap(){
			opts[DRAW_EDGES] = true;
		};

		~OptionsMap(){};

		static OptionsMap* instance;
};

#endif
