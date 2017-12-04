#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_
#include "collidable.h"
#include "scene_node.h"
#include "ray.h"

namespace game {
	// Class that manages one object in a scene 
	class CollisionManager {
	public:
		static bool isColliding(Collidable* a, Collidable* b);
		static bool isColliding(Collidable* n, Ray r);
		static bool checkHierarchicalCollision(SceneNode* a, SceneNode* b);
		static bool checkHierarchicalCollision(SceneNode* n, Ray r);

	private:
		CollisionManager();

		static bool isColliding(AABB a, AABB b);
		static bool isColliding(Hitbox a, Hitbox b);
		static bool isColliding(AABB a, Ray r);
		static bool isColliding(Hitbox a, Ray r, glm::vec2** intesection);
		static glm::vec3 rotateAxis(glm::vec3 v, glm::mat4 t);
		static std::vector<SceneNode*> flattenTree(SceneNode* root);
	};

} // game


#endif // COLLISION_MANAGER_H_