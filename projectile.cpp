#include "projectile.h"

namespace game {
	int Projectile::ID = 0;

	Projectile::Projectile(std::string par, glm::vec3 p, glm::vec3 v, glm::vec3 a, float dam, const Resource* geom, const Resource* mat, const Resource* tex)
		: AttackNode("Projectile" + std::to_string(ID++), dam, geom, mat, tex) {
		SetPosition(p);
		vel = v;
		accel = a;
		parent_name = par;

		SetOrientation(VectorToRotation(v));

	}

	std::string Projectile::GetParentName() {
		return parent_name;
	}

	void Projectile::Update(float t) {
		vel += accel*t;
		Translate(vel*10.0f*t);

		lifetime += t;
		if (lifetime > lifespan) {
			destroy();
		}
			
		glm::quat rotation = glm::slerp(GetOrientation(), VectorToRotation(vel), 0.5f);
		SetOrientation(rotation);

	}
}