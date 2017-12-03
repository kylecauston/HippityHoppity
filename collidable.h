#ifndef COLLIDABLE_H_
#define COLLIDABLE_H_
#include "hitbox.h"
#include "AABB.h"

namespace game {
	class Collidable {
	public:
		Hitbox hb;
		AABB aabb;
		
		virtual void updateCollidable(glm::mat4 transf);
		virtual void onCollide(Collidable* other) = 0;
	};

} // game


#endif // COLLIDABLE_H_