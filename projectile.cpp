#include "projectile.h"

namespace game {
	int Projectile::ID = 0;

	Projectile::Projectile(glm::vec3 p, glm::vec3 v, glm::vec3 a, const Resource* geom, const Resource* mat, const Resource* tex)
		: AttackNode("Projectile" + std::to_string(ID++), geom, mat, tex) {
		SetPosition(p);
		vel = v;
		accel = a;
	}

	void Projectile::Update(float t) {
		vel += accel*t;
		Translate(vel*100.0f*t);
	}
}