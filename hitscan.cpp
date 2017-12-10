#include "hitscan.h"

namespace game {
	int Hitscan::ID = 0;

	Hitscan::Hitscan(Ray _r, float dam) : AttackNode("Hitscan" + std::to_string(ID++), dam, NULL, NULL, NULL)
	{
		r = _r;
	}

	Hitscan::Hitscan(glm::vec3 o, glm::vec3 d, float dam) : Hitscan(Ray(o, d), dam) {}

	Ray Hitscan::getRay(void) const {
		return r;
	}
}