#ifndef PROJECTILE_H_
#define PROJECTILE_H_
#include "attack_node.h"

namespace game {
	class Projectile : public AttackNode {
		// class to represent projectile attack styles
	public:
		Projectile(std::string par, glm::vec3 p, glm::vec3 v, glm::vec3 a, float dam, const Resource *geometry, const Resource *material, const Resource *tex = NULL);

		void Update(double d);
		std::string GetParentName();

	private:
		static int ID;
		const float lifespan = 5; // seconds until projectile expiry
		float lifetime = 0;
		glm::vec3 vel, accel;
		int maxSpeed = -1;
		std::string parent_name; // what entity spawned this
	};
}
#endif  // PROJECTILE_H