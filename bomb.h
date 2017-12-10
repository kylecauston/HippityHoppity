#ifndef BOMB_H_
#define BOMB_H_
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include "resource.h"
#include "scene_node.h"

namespace game {
	class Bomb : public SceneNode {
	public:
		Bomb(const std::string name, const Resource *geometry, const Resource *material, double ttl = 4.0,
			glm::vec3 rgb = glm::vec3(0.8, 0.8, 0.8), const Resource *tex = NULL);

		// Destructor
		~Bomb();

		glm::vec3 GetDir(void) const;
		void SetDir(glm::vec3 dir);

		// Update geometry configuration
		void Update(double delta_time);
		glm::mat4 Draw(Camera * camera, glm::mat4 parent_transf, bool sun);
		glm::mat4 SetupShader(GLuint program, glm::mat4 parent_transf, bool sun);
		glm::vec3 rgb_col;

	private:
		glm::vec3 direction;
		float speed = 3.0;
	}; // class

} // namespace game

#endif // BOMB _H_
