#ifndef RAY_H_
#define RAY_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace game {
	class Ray {
	private:
		glm::vec3 origin;
		glm::vec3 direction; // unit vector

	public:
		Ray(glm::vec3 o, glm::vec3 d);

		glm::vec3 getOrigin();
		glm::vec3 getDirection();
		glm::vec3 getPoint(float t);

		};

} // game


#endif // RAY_H_