#include "mole.h"

namespace game {
	float Mole::damage = 1.5;

	Mole::Mole(const std::string name, SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex)
		: Enemy(name, targ, geometry, mat, tex)
	{
		speed = 0;
		rotateSpeed = 0.1;
		firerate = 0.5;
	}

	Mole::~Mole() {}

	void Mole::Update(double deltaTime) {
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
	}

	AttackNode* Mole::getAttack() {
		// Mole attack is a hitscan sniper
		resetCooldown();

		glm::vec3 aim = GetOrientation() * SceneNode::default_forward;
		Ray r = Ray(GetAbsolutePosition(), aim);
		AttackNode* shot = new Hitscan(r, damage);

		//shot->SetScale(0.1, 0.1, 0.1);
		return shot;
	}
}