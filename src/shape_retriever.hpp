#ifndef __SHAPE_RETRIEVER_HPP__
#define __SHAPE_RETRIEVER_HPP__

#include "mesh.hpp"

typedef std::shared_ptr<Mesh> MeshPtr;

namespace Retriever {

	enum class DistanceMethod {
		eucliden_NoWeights = 0,
		quadratic_Weights = 1,
		flat_NoWeights = 2,
		emd_NoWeights = 3,
		spotify_ANN = 4
	};
	void retrieveSimiliarShapesCUST(const MeshPtr& mesh, std::filesystem::path dbPath, bool includeSelf, std::array<float, 6> scalarWeights, std::array<float, 6> functionWeights, bool squareDistance, bool useEMD, bool useSqrt);
	void retrieveSimiliarShapes(const MeshPtr& mesh, std::filesystem::path dbPath, int shapes, DistanceMethod method, bool includeSelf = false);
	void retrieveSimiliarShapesANN(const MeshPtr& mesh, std::filesystem::path dbPath, int shapes, bool includeSelf = false);
}

#endif
