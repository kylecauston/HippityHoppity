#include "aabb.h"

namespace game {
	AABB::AABB(glm::vec3 p, glm::vec3 s) {
		pos = p;
		scale = s;
	}

	AABB::AABB() {}

	AABB::~AABB() {}

	glm::vec3 AABB::getPos() { return pos; }
	glm::vec3 AABB::getScale() { return scale; }
}