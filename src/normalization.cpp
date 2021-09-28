#include "normalization.hpp"

namespace Normalization {
	void scale(Eigen::MatrixXf& V) {
		Eigen::Vector3f min = V.colwise().minCoeff();
		Eigen::Vector3f max = V.colwise().maxCoeff();

		Eigen::Vector3f diff = max - min;
		diff = diff.array().abs();
		auto scaleFactor = 1.0f / diff.maxCoeff();
		V = V * scaleFactor;
	}

	Eigen::Matrix3f calculateCovarianceMatrix(const Eigen::MatrixXf& V, const Eigen::Vector3f& centroid) {
		Eigen::Matrix3f covarianceMatrix = Eigen::Matrix3f::Identity();

		// Could be optimized by not looping and explicitly setting 0,1 and 1,0 at the same time for example (symmetric)
		// Probably completely pointless though
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				float sum = 0;
				for (int index = 0; index < V.rows(); index++) {
					sum += (V.row(index)[i] - centroid[i]) * (V.row(index)[j] - centroid[j]);
				}
				sum /= (V.rows() - 1);
				covarianceMatrix.row(i)[j] = sum;
			}
		}

		return covarianceMatrix;
	}

	std::vector<std::pair<Eigen::Vector3f, float>> calculateEigen(const Eigen::Matrix3f& covarianceMatrix) {

		Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigenSolver(covarianceMatrix);
		Eigen::Vector3f eigenVector1 = eigenSolver.eigenvectors().col(0);
		Eigen::Vector3f eigenVector2 = eigenSolver.eigenvectors().col(1);
		Eigen::Vector3f eigenVector3 = eigenSolver.eigenvectors().col(2);

		float lambda1 = eigenSolver.eigenvalues()[0];
		float lambda2 = eigenSolver.eigenvalues()[1];
		float lambda3 = eigenSolver.eigenvalues()[2];

		return std::vector<std::pair<Eigen::Vector3f, float>>{
			std::make_pair(eigenVector1, lambda1),
			std::make_pair(eigenVector2, lambda2),
			std::make_pair(eigenVector3, lambda3)
		};
	}

	void alignPrincipalAxes(Eigen::MatrixXf& V, const Eigen::Vector3f& centroid, const Eigen::Vector3f& majorEigenVector, const Eigen::Vector3f& minorEigenVector) {
		for (int index = 0; index < V.rows(); index++) {
			Eigen::Vector3f originalPoint = V.row(index);
			Eigen::Vector3f diff = originalPoint - centroid;
			auto xPos = diff.dot(majorEigenVector);
			auto yPos = diff.dot(minorEigenVector);
			auto zPos = diff.dot(majorEigenVector.cross(minorEigenVector));
			V.row(index) = Eigen::Vector3f(xPos, yPos, zPos);
		}
	}

	void flipMirrorTest(Eigen::MatrixXf& V, const Eigen::MatrixXi& F) {
		float xSum = 0;
		float ySum = 0;
		float zSum = 0;

		for (int f = 0; f < F.rows(); f++) {

			auto point1 = V.row(F(f, 0));
			auto point2 = V.row(F(f, 1));
			auto point3 = V.row(F(f, 2));

			auto centroidX = (point1.x() + point2.x() + point3.x()) / 3;
			auto centroidY = (point1.y() + point2.y() + point3.y()) / 3;
			auto centroidZ = (point1.z() + point2.z() + point3.z()) / 3;

			auto centroidXSquared = centroidX * centroidX;
			auto centroidYSquared = centroidY * centroidY;
			auto centroidZSquared = centroidZ * centroidZ;
			if (centroidX < 0) {
				centroidXSquared *= -1;
			}

			if (centroidY < 0) {
				centroidYSquared *= -1;
			}

			if (centroidZ < 0) {
				centroidZSquared *= -1;
			}

			xSum += centroidXSquared;
			ySum += centroidYSquared;
			zSum += centroidZSquared;
		}

		Eigen::Matrix3f flipMatrix = Eigen::Matrix3f::Identity();
		flipMatrix.row(0)[0] = sgn(xSum);
		flipMatrix.row(1)[1] = sgn(ySum);
		flipMatrix.row(2)[2] = sgn(zSum);

		for (int index = 0; index < V.rows(); index++) {
			V.row(index) *= flipMatrix;
		}
	}
}
