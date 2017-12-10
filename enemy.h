#ifndef ENEMY_H_
#define ENEMY_H_
#include "scene_node.h"
#include "collidable.h"
#include "attack_node.h"
#include "hitscan.h"
#include "projectile.h"

namespace game {
	// Class that manages one object in a scene 
	class Enemy : public SceneNode {
	public:
		// Create enemy from given resources
		Enemy(const std::string name, SceneNode* targ, const Resource *geometry, const Resource *material, const Resource *tex = NULL);

		// Destructor
		~Enemy();

		float getRotateSpeed();
		float getMovementSpeed();
		bool isAttacking();
		virtual AttackNode* getAttack() = 0;

		void setRotateSpeed(float s);
		void setMovementSpeed(float s);
		void setFirerate(float f);
		void setProjectileGeometry(Resource* g);
		void setProjectileMaterial(Resource* m);
		void setProjectileTexture(Resource* t);

		// Update: move, attack etc
		void Update(double deltaTime);

		void collide(Collidable* other);

	protected:
		SceneNode* target = NULL; // what this enemy is trying to destroy

		void resetCooldown();

		float threatRange; // radius that enemy begins attacking from

		float rotateSpeed = 0.5; // factor for rotation speed [0, 1]
		float speed = 6.0; // how fast the enemy moves per second

		float firerate = 1; // how many shots per second
		float shot_CD = 0; // how long until this can shoot
		bool attack_flag = false; // does this enemy want to attack

		Resource* projectile_geometry = NULL;
		Resource* projectile_material = NULL;
		Resource* projectile_texture = NULL;
	};
} // game
#endif // SCENE_NODE_H_