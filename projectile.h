#ifndef PROJECTILE_H_
#define PROJECTILE_H_

#include "attack_node.h"

namespace game {
	class Projectile : public AttackNode {
		// class to represent projectile attack styles
	public:
		Projectile(glm::vec3 p, glm::vec3 v, glm::vec3 a, const Resource *geometry, const Resource *material, const Resource *tex = NULL);

		void Update(double d);

	private:
		static int ID;
		glm::vec3 vel, accel;
	};
}

#endif  // PROJECTILE_H