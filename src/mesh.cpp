#include "mesh.hpp"

Mesh::Mesh(std::filesystem::path path) : m_meshPath(path) {
	if (!Importer::importModel(path, V, F)) {
		return;
	}
	descriptors = std::make_shared<Descriptors>(V, F);
	m_convexHull = std::make_shared<ConvexHull>(V);
}

void Mesh::writeMesh() {
	Exporter::exportModel(m_meshPath, V, F);
}


