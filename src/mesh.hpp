#ifndef __MESH_HPP__
#define __MESH_HPP__
#include "mesh_base.hpp"
#include "convex_hull.hpp"
#include <filesystem>

class Mesh :public MeshBase {
	public:
		Mesh(std::filesystem::path path);
		~Mesh() {};

		inline std::filesystem::path getPath() const { return m_meshPath; }
		inline std::shared_ptr<ConvexHull> getConvexHull() { return (m_convexHull) ? m_convexHull : (m_convexHull = std::make_shared<ConvexHull>(m_vertices)); }
		inline std::vector<std::pair<std::string, float>> getSimilarShapes() { return m_similarShapes; }
		inline void setSimilarShapes(std::vector<std::pair<std::string, float>> similarShapes) { m_similarShapes = similarShapes; }

		void writeMesh();
		void writeMesh(std::filesystem::path filePath);
		void prepare() override;
		void unprepare() override;
		void update(float dt) override;
		void recomputeAndRender() override;
		void resetTransformations() override;

	private:

		std::filesystem::path m_meshPath;
		std::shared_ptr<ConvexHull> m_convexHull;
		std::vector<std::pair<std::string, float>> m_similarShapes;
};

#endif
