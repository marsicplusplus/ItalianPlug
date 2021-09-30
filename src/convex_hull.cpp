#include "convex_hull.hpp"
ConvexHull::ConvexHull(Eigen::MatrixXf meshV) {

	// To-Do compute convex hull
	descriptors = std::make_shared<Descriptors>(V, F);
}
