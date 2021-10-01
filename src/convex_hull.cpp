#define QUICKHULL_IMPLEMENTATION

#include "convex_hull.hpp"
#include "shader_map.hpp"
#include "QuickHull.hpp"

ConvexHull::ConvexHull(Eigen::MatrixXf vertices) {
	computeConvexHull(vertices);
}

void ConvexHull::draw(const glm::mat4& projView, const glm::vec3& edgeColor, const glm::vec3& cameraPos) {
	glBindVertexArray(m_VAO);
	unsigned int drawMode = GL_FILL;
	switch (OptionsMap::Instance()->getOption(DRAW_MODE)) {
		case POINT_CLOUD:
			ShaderMap::Instance()->getShader(SHADER_EDGE)->use();
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("projView", projView);
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("model", m_modelMatrix);
			drawMode = GL_POINT;
			break;
		case WIREFRAME:
		default:
			ShaderMap::Instance()->getShader(SHADER_EDGE)->use();
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("projView", projView);
			ShaderMap::Instance()->getShader(SHADER_EDGE)->setUniform("model", m_modelMatrix);
			drawMode = GL_LINE;
			break;
	};
	glPolygonMode(GL_FRONT_AND_BACK, drawMode);
	glDrawElements(GL_TRIANGLES, 3 * m_faces.rows(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void ConvexHull::computeConvexHull(Eigen::MatrixXf vertices) {
	using namespace quickhull;
	QuickHull<float> qh; // Could be double as well
	std::vector<Vector3<float>> pointCloud;
	for (int i = 0; i < vertices.rows(); i++) {
		const auto vertex = vertices.row(i);
		pointCloud.push_back(Vector3(vertex.x(), vertex.y(), vertex.z()));
	}

	// Add points to point cloud
	auto hull = qh.getConvexHull(pointCloud, true, false);
	const auto& indexBuffer = hull.getIndexBuffer();
	const auto& vertexBuffer = hull.getVertexBuffer();

	m_vertices.conservativeResize(vertexBuffer.size(), 3);
	m_faces.conservativeResize(indexBuffer.size() / 3, 3);

	for (int i = 0; i < vertexBuffer.size(); i++) {
		m_vertices.row(i) = Eigen::Vector3f(vertexBuffer[i].x, vertexBuffer[i].y, vertexBuffer[i].z);
	}

	for (int i = 0; i < indexBuffer.size() / 3; i++) {
		m_faces.row(i) = Eigen::Vector3i(indexBuffer[i * 3], indexBuffer[i * 3 + 1], indexBuffer[i * 3 + 2]);
	}
}
