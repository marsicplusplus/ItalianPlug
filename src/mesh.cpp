#include "mesh.hpp"

Mesh::Mesh(std::filesystem::path path) : m_meshPath(path) {
	if (!Importer::importModel(path, m_vertices, m_faces)) {
		return;
	}
	m_convexHull = std::make_shared<ConvexHull>(m_vertices);
}

void Mesh::writeMesh() {
	writeMesh(m_meshPath);
}

void Mesh::writeMesh(std::filesystem::path filePath) {
	Exporter::exportModel(filePath, m_vertices, m_faces);
}


void Mesh::prepare() {
	MeshBase::prepare();
	m_convexHull->prepare();
}

void Mesh::unprepare() {
	MeshBase::unprepare();
	m_convexHull->unprepare();
}

void Mesh::update(float dt) {
	MeshBase::update(dt);
	m_convexHull->update(dt);
}

void Mesh::recomputeAndRender() {
	MeshBase::recomputeAndRender();
	m_convexHull->computeConvexHull(m_vertices);
	m_convexHull->recomputeAndRender();
}

void Mesh::resetTransformations() {
	MeshBase::resetTransformations();
	m_convexHull->resetTransformations();
}

