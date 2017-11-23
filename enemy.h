#ifndef ENEMY_H_
#define ENEMY_H_
#include "scene_node.h"

namespace game {
	// Class that manages one object in a scene 
	class Enemy : public SceneNode {
	public:
		// Create asteroid from given resources
		Enemy(const std::string name, const SceneNode* targ, const Resource *geometry, const Resource *material, const Resource *tex = NULL);

		// Destructor
		~Enemy();

		// Update: move, attack etc
		void Update(float deltaTime);

		// attack target if possible
		void Attack(void);

		float rotateSpeed = 0.3; // factor for rotation speed [0, 1]
	private:
		const SceneNode* target; // what this enemy is trying to destroy
		float speed = 10.0; // how fast the enemy moves per second
		float threatRange; // radius that enemy begins attacking from

		float firerate; // how many shots per second

		glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest);
	};

} // game


#endif // SCENE_NODE_H_