#ifndef HITBOX_H_
#define HITBOX_H_
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/matrix_access.hpp"

namespace game {
	class Hitbox {
	private:
		glm::vec3 scale;
		glm::mat4 trans;
		glm::vec3 pos;

		std::vector<glm::vec3> base_points;

	public:
		Hitbox(std::vector<glm::vec3> p);
		Hitbox();
		~Hitbox();

		// returns the 8 points made by this hitbox
		std::vector<glm::vec3> getPoints();

		glm::vec3 getScale();
		glm::vec3 getPos();
		glm::mat4 getTrans();

		void setScale(glm::vec3 s);
		void setTransform(glm::mat4 t);
	};
}
#endif // HITBOX_H_