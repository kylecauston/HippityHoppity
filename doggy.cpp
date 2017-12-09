#include "doggy.h"
#include <iostream>

namespace game {
	Doggy::Doggy(const std::string name, SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex)
		: Enemy(name, targ, geometry, mat, tex)
	{
		turret = NULL;
	}

	Doggy::~Doggy() {

	}

	void Doggy::setTurret(SceneNode* t) {
		turret = t;
	}

	AttackNode* Doggy::getAttack() {
		// Doggy attack is an arcing projectile (maybe)
		resetCooldown();

		glm::vec3 aim = turret->GetOrientation() * SceneNode::default_forward;
		/*Ray r = Ray(GetAbsolutePosition(), aim);
		AttackNode* shot = new Hitscan(r);*/
		AttackNode* shot = new Projectile(GetEntityName(), GetEntityPosition(), aim*10.0f, glm::vec3(0, -0.05, 0), 1.0,
			projectile_geometry, projectile_material, projectile_texture);

		shot->SetScale(0.3, 0.3, 0.3);

		return shot;
	}

	void Doggy::Update(float t) {
		Enemy::Update(t);

		if (target == NULL) 
			return;

		glm::vec3 toTarget = target->GetPosition() - GetEntityPosition();
		// 2D to target vector, ignoring height
		glm::vec3 toTarget_flat = glm::vec3(toTarget.x, 0, toTarget.z);

		

		if (glm::length(toTarget_flat) != 0) {
			glm::quat body_rotation = glm::slerp(GetOrientation(), VectorToRotation(toTarget_flat), 0.05f);
			SetOrientation(body_rotation);

			glm::quat turret_rotation = glm::slerp(turret->GetOrientation(), VectorToRotation(toTarget), rotateSpeed);
			turret->SetOrientation(turret_rotation);
		}
		
		// this moves the dog towards target
		// TODO: switch to movement in the direction of facing
		if (glm::length(toTarget_flat) > t*speed) {

			// we drop the y value if the enemy can't fly, and then make 
			// the vector into a movement vector (the actual units it will move)
			glm::vec3 movement = (t*speed) * (glm::normalize(toTarget_flat));

			// move the parent node (the dog has a dummy parent to keep turret separate)
			parent_->Translate(movement);
		}
		
	}

}