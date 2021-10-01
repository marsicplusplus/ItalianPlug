#include "mesh.hpp"

Mesh::Mesh(std::filesystem::path path) : m_meshPath(path) {
	if (!Importer::importModel(path, m_vertices, m_faces)) {
		return;
	}

	m_descriptors = std::make_shared<Descriptors>(m_vertices, m_faces);
	m_convexHull = std::make_shared<ConvexHull>(m_vertices);
}

void Mesh::writeMesh() {
	Exporter::exportModel(m_meshPath, m_vertices, m_faces);
}

void Mesh::prepare() {
	MeshBase::prepare();
	m_convexHull->prepare();
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

void Mesh::computeFeatures(){
	Descriptors::computeDescriptors(V, F, Descriptors::descriptor_all, features);
}

float Mesh::getDescriptor(Features f) {
	auto t = features.find(f);
	if(t == features.end()) return 0.0f;
	else return t->second;
}
