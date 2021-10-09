#ifndef __MESH_REPAIR_HPP__
#define __MESH_REPAIR_HPP__

#include "Eigen/Dense"

using namespace Eigen;

namespace MeshRepairer{

	void repairMesh(MatrixXd V, MatrixXi F, MatrixXd& finalV, MatrixXi& finalF);
}

#endif