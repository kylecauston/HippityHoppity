#include "cat.h"

namespace game {
	float Cat::damage = 0.5;

	Cat::Cat(const std::string name, SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex)
		: Enemy(name, targ, geometry, mat, tex)
	{
		speed = 30.0;
		rotateSpeed = 0.1;
	}

	Cat::~Cat() {}

	void Cat::Update(double deltaTime) {
		// The mole enemy rotates to face the target, but doesn't move
		Enemy::Update(deltaTime);

		if (target == NULL)
			return;

		// get the vector towards target
		glm::vec3 toTarget = target->GetPosition() - GetEntityPosition();

		if (glm::length(toTarget) != 0) {
			glm::quat rotation = glm::slerp(GetOrientation(), VectorToRotation(toTarget), rotateSpeed);
			SetOrientation(rotation);
		}

		if (glm::length(toTarget) > 100 && glm::length(toTarget) > deltaTime*speed) {
			// the vector into a movement vector (the actual units it will move)
			glm::vec3 movement = ((float)deltaTime *speed) * (glm::normalize(toTarget));

			// move the parent node (the dog has a dummy parent to keep turret separate)
			Translate(movement);
		}
	}

	AttackNode* Cat::getAttack() {
		resetCooldown();

		glm::vec3 aim = GetOrientation() * SceneNode::default_forward;
		/*Ray r = Ray(GetAbsolutePosition(), aim);
		AttackNode* shot = new Hitscan(r);*/
		AttackNode* shot = new Projectile(GetEntityName(), GetEntityPosition(), aim*10.0f, glm::vec3(0, -0.05, 0), 1.0,
			projectile_geometry, projectile_material, projectile_texture);

		shot->SetScale(0.3, 0.3, 0.3);

		return shot;
	}
}