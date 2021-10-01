#ifndef __CONVEX_HULL_HPP__
#define __CONVEX_HULL_HPP__

#include "mesh_base.hpp"

class ConvexHull :public  MeshBase {
	public:
		ConvexHull(Eigen::MatrixXf vertices);
		~ConvexHull() {};

		void computeConvexHull(Eigen::MatrixXf vertices);
		void draw(const glm::mat4& projView, const glm::vec3& edgeColor, const glm::vec3& cameraPos) override;

};

#endif
