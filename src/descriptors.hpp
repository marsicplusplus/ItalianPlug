#pragma once 

#include "Eigen/Dense"

class Descriptors {
	public:
		Descriptors();
		~Descriptors();

		float getArea() { return m_area; }
		float getMeshVolume() { return m_meshVolume; }
		float getBoundingBoxVolume() { return m_boundingBoxVolume; }
		float getDiameter() { return m_diameter; }
		float getCompactness() { return m_compactness; }
		float getEccentricity() { return m_eccentricity; }
		int get2DArea() { return m_2dArea; }
		int get2DPerimeter() { return m_2dPerimeter; }

		void compute3DDescriptors(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F, unsigned int flags);
		void compute2DDescriptors(uint8_t* fb, int wW, int wH, unsigned int flags);
		void computeArea(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeMeshVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeBoundingBoxVolume(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeCompactness();
		void computeEccentricity(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void computeDiameter(const Eigen::MatrixXf& V, const Eigen::MatrixXi& F);
		void compute2DArea(uint8_t* fb, int wW, int wH);
		void compute2DPerimeter(uint8_t* fb, int wW, int wH);

		enum descriptors3D : unsigned int
		{
			descriptor_area					= 1 << 0,
			descriptor_meshVolume			= 1 << 1,
			descriptor_boundingBoxVolume	= 1 << 2,
			descriptor_diameter				= 1 << 3,
			descriptor_compactness			= 1 << 4,
			descriptor_eccentricity			= 1 << 5,
			descriptor3d_all					= 255 
		};
		enum descriptors2D : unsigned int
		{
			descriptor_2Darea				= 1 << 0,
			descriptor_2Dperimeter 			= 1 << 1,
			descriptor2d_all					= 255
		};

	private:
		float m_area;
		float m_meshVolume;
		float m_boundingBoxVolume;
		float m_diameter;
		float m_compactness;
		float m_eccentricity;
		int m_2dArea;
		int m_2dPerimeter;
};

