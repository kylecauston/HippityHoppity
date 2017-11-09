#ifndef LASER_H_
#define LASER_H_

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

	class Laser : public SceneNode {

	public:
		Laser(const std::string name, const Resource *geometry, const Resource *material);

		// Destructor
		~Laser();

		glm::vec3 GetDir(void) const;
		void SetDir(glm::vec3 dir);

		// Update geometry configuration
		void Update(void);

	private:
		glm::vec3 direction;
		float speed = 5.0;
	}; // class

} // namespace game

#endif // LASER_H_
