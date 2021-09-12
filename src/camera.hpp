#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include "glm/glm.hpp"

class Camera {
	public:
		Camera(glm::vec3 pos, glm::vec3 up, float fov, float yaw, float pitch, float roll);
		~Camera();

		glm::mat4 getViewMatrix() const;
		float getFOV() const;
		glm::vec3 getPosition() const;

		void update(float dt);

	protected:
		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 right;
		glm::vec3 up;
		glm::vec3 worldUp;

		float fov;
		float yaw;
		float roll;
		float pitch;

		void updateVectors();
};

#endif
