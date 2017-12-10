#include "enemy.h"
#include <iostream>

namespace game {
	Enemy::Enemy(const std::string name, SceneNode* targ, const Resource *geometry, const Resource *material, const Resource *tex) : SceneNode(name, geometry, material, tex, true) {
		target = targ;
	}

	Enemy::~Enemy() {}

	float Enemy::getRotateSpeed() {
		return rotateSpeed;
	}

	float Enemy::getMovementSpeed() {
		return speed;
	}

	bool Enemy::isAttacking() {
		return attack_flag;
	}

	void Enemy::setRotateSpeed(float s) {
		rotateSpeed = s;
	}

	void Enemy::setMovementSpeed(float s) {
		speed = s;
	}

	void Enemy::setFirerate(float f) {
		firerate = f;
	}

	void Enemy::setProjectileGeometry(Resource* g) {
		projectile_geometry = g;
	}

	void Enemy::setProjectileMaterial(Resource* m) {
		projectile_material = m;
	}

	void Enemy::setProjectileTexture(Resource* t) {
		projectile_texture = t;
	}

	void Enemy::Update(double t) {
		float deltaTime = t;

		if (target && target->isDestroyed()) {
			target = NULL;
		}

		if (shot_CD <= 0) {
			if (target) {
				attack_flag = true;
			}
		}
		else {
			shot_CD -= deltaTime;
		}
	}

	void Enemy::collide(Collidable* other)
	{

	}

	void Enemy::resetCooldown() {
		// reset attack timer
		attack_flag = false;

		if (firerate == 0) {
			shot_CD = INFINITY;
		}
		else {
			shot_CD = 1 / firerate;
		}
	}
} // namespace game
