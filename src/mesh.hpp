#pragma once
#include "mesh_base.hpp"
#include "convex_hull.hpp"

class Mesh :public MeshBase {
	public:
		Mesh(std::filesystem::path path);
		~Mesh() {};

		inline std::filesystem::path getPath() const { return m_meshPath; }
		void writeMesh();
		void writeMesh(std::filesystem::path filePath);
		inline std::shared_ptr<ConvexHull> getConvexHull() const { return m_convexHull; }


	private:

		std::filesystem::path m_meshPath;
		std::shared_ptr<ConvexHull> m_convexHull;
};

