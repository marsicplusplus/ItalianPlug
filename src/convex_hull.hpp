#pragma once

#include "mesh_base.hpp"

class ConvexHull :public  MeshBase {
	public:
		ConvexHull(Eigen::MatrixXf meshV);
		~ConvexHull() {};
};

