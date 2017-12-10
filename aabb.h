#ifndef AABB_H_
#define AABB_H_
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace game {
	// Class that manages one object in a scene 
	class AABB {
	private:
		glm::vec3 pos;
		glm::vec3 scale;
	public:
		AABB(glm::vec3 p, glm::vec3 s);
		AABB();
		~AABB();

		glm::vec3 getPos();
		glm::vec3 getScale();
	};
} // game
#endif // AABB_H_