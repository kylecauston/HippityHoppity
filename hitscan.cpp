#include "hitscan.h"

namespace game {
	int Hitscan::ID = 0;

	Hitscan::Hitscan(Ray _r) : AttackNode("Hitscan" + std::to_string(ID++), NULL, NULL, NULL)
	{
		r = _r;
	}

	Hitscan::Hitscan(glm::vec3 o, glm::vec3 d) : Hitscan(Ray(o, d)) {}

	Ray Hitscan::getRay(void) const {
		return r;
	}
}