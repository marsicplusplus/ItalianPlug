#ifndef __MESH_MANAGER_HPP__
#define __MESH_MANAGER_HPP__

#include <algorithm>
#include <unordered_map>
#include <memory>
#include "mesh.hpp"
#include "utils.hpp"

typedef std::shared_ptr<Mesh> MeshPtr;

class MeshMap {
	public:
		static MeshMap *Instance(){
			if(instance == nullptr){
				instance = new MeshMap();
			}
			return instance;
		}

		inline MeshPtr getMesh(std::string path) {
			std::size_t h = std::hash<std::string>{}(path);
			auto mesh = map.find(h);
			if(mesh == map.end()) {
				MeshPtr mesh = std::make_shared<Mesh>(path);
				map.insert(std::make_pair(h, mesh));
				return mesh;
			}
			else 
				return mesh->second;
		}

		inline void destroy(){
			map.clear();
			delete instance;
		}

	private:
		MeshMap() {};
		~MeshMap() {};

		std::unordered_map<std::size_t, MeshPtr> map;
		static MeshMap* instance;
};

typedef MeshMap _MeshMap;

#endif
