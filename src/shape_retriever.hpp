#ifndef __SHAPE_RETRIEVER_HPP__
#define __SHAPE_RETRIEVER_HPP__

#include "mesh.hpp"

typedef std::shared_ptr<Mesh> MeshPtr;

namespace Retriever {
	void retrieveSimiliarShapes(const MeshPtr& mesh, std::filesystem::path dbPath, bool includeSelf = false);
	void retrieveSimiliarShapesKNN(const MeshPtr& mesh, std::filesystem::path dbPath, int shapes, bool includeSelf = false);
}

#endif
