#include "enemy.h"
#include <iostream>

namespace game {
	Enemy::Enemy(const std::string name, SceneNode* targ, const Resource *geometry, const Resource *material, const Resource *tex) : SceneNode(name, geometry, material, tex, true) {
		target = targ;
	}

	Enemy::~Enemy() {}

	void Enemy::Update(float deltaTime) {
		if (target == NULL)
			return;

		// get the plane vector towards target
		glm::vec3 toTarget = target->GetPosition() - GetEntityPosition();
		// 2D to target vector, ignoring height
		glm::vec3 toTarget_flat = glm::vec3(toTarget.x, 0, toTarget.z);

		if (glm::length(toTarget_flat) > deltaTime*speed) {

			// we drop the y value if the enemy can't fly, and then make 
			// the vector into a movement vector (the actual units it will move)
			glm::vec3 movement = (deltaTime*speed) * (glm::normalize(toTarget));

			Translate(movement);
		}

		toTarget = target->GetPosition() - GetEntityPosition();
		toTarget_flat = glm::vec3(toTarget.x, 0, toTarget.z);


		if (glm::length(toTarget_flat) > 1) {
			// rotate on the y axis (turn towards where the target is in 2D space)
			// make a triangle from target and self pos, take angle, rotate
			float angle = asin(toTarget.x / sqrt(pow(toTarget.x, 2) + pow(toTarget.z, 2)));
			if (toTarget.z < 0)
			{
				angle = glm::pi<float>() - angle;
			}

			glm::quat hor_rotation = glm::angleAxis(angle, glm::vec3(0.0, 1.0, 0.0));

			// now rotate upwards to look at target

			// get the angle to aim upwards (no more than 90deg)
			float vert_angle = asin(toTarget.y / glm::length(toTarget));

			glm::quat vert_rotation = glm::angleAxis(vert_angle, glm::cross(SceneNode::default_forward, glm::vec3(0.0, 1.0, 0.0)));
			glm::quat rotation = glm::slerp(GetOrientation(), hor_rotation*vert_rotation, rotateSpeed);
			SetOrientation(rotation);

			// may need to subtract any parent rotations somehow 
			//   or simply find a way to set strict rotations that aren't cumulative
		}

		// if in range, attack
		
		if (shot_CD <= 0) {
			attack_flag = true;
		}
		else {
			shot_CD -= deltaTime;
		}
		
	}

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
	
	AttackNode* Enemy::getAttack() {
		attack_flag = false;

		if (firerate == 0) {
			shot_CD = INFINITY;
		}
		else {
			shot_CD = 1 / firerate;
		}

		glm::vec3 aim = GetOrientation() * SceneNode::default_forward;
		/*Ray r = Ray(GetAbsolutePosition(), aim);
		AttackNode* shot = new Hitscan(r);*/
		AttackNode* shot = new Projectile(GetEntityPosition(), aim, glm::vec3(0, -0.1, 0), 
			projectile_geometry, projectile_material, projectile_texture);

		//shot->SetScale(10, 10, 10);

		return shot;
	}

	void Enemy::collide(Collidable* other)
	{
		
	}
} // namespace game
