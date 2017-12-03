#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_
#include "collidable.h"
#include "scene_node.h"

namespace game {
	// Class that manages one object in a scene 
	class CollisionManager {
	public:
		static bool isColliding(Collidable* a, Collidable* b);
		static bool checkHierarchicalCollision(SceneNode* a, SceneNode* b);

	private:
		CollisionManager();

		static bool isColliding(AABB a, AABB b);
		static bool isColliding(Hitbox a, Hitbox b);
		static glm::vec3 rotateAxis(glm::vec3 v, glm::mat4 t);
	};

} // game


#endif // COLLISION_MANAGER_H_