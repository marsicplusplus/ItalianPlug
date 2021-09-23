#include "Eigen/Dense"
#include "utils.hpp"
#include "shader.hpp"

class UnitCube {

public:
	UnitCube();
	~UnitCube();

	void draw(const glm::mat4& projView);

private:
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	float vertices[24] = {
		// front
		-0.5, -0.5,  0.5,
		 0.5, -0.5,  0.5,
		 0.5,  0.5,  0.5,
		-0.5,  0.5,  0.5,
		// back
		-0.5, -0.5, -0.5,
		 0.5, -0.5, -0.5,
		 0.5,  0.5, -0.5,
		-0.5,  0.5, -0.5
	};
	/* init_resources */
	unsigned int indices[36] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	Shader shader;
};
