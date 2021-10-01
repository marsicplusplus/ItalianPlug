#ifndef __MESH_HPP__
#define __MESH_HPP__
#include "mesh_base.hpp"
#include "convex_hull.hpp"

class Mesh :public MeshBase {
	public:
		Mesh(std::filesystem::path path);
		~Mesh() {};

		inline std::filesystem::path getPath() const { return m_meshPath; }
		inline std::shared_ptr<ConvexHull> getConvexHull() { return (m_convexHull) ? m_convexHull : (m_convexHull = std::make_shared<ConvexHull>(m_vertices)); }

		void writeMesh();
		void prepare() override;
		void update(float dt) override;
		void recomputeAndRender() override;
		void resetTransformations() override;

	private:

		std::filesystem::path m_meshPath;
		std::shared_ptr<ConvexHull> m_convexHull;
};

#endif
