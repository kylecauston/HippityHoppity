#include "doggy.h"
#include <iostream>

namespace game {
	Doggy::Doggy(const std::string name, const SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex)
		: Enemy(name, targ, geometry, mat, tex)
	{
		turret = NULL;
	}

	Doggy::~Doggy() {

	}

	void Doggy::setTurret(SceneNode* t) {
		turret = t;
	}

	void Doggy::Update(float t) {
		//Enemy::Update(t);

		glm::vec3 toTarget = target->GetPosition() - GetAbsolutePosition();
		// 2D to target vector, ignoring height
		glm::vec3 toTarget_flat = glm::vec3(toTarget.x, 0, toTarget.z);

		if (glm::length(toTarget_flat) > t*speed) {

			// we drop the y value if the enemy can't fly, and then make 
			// the vector into a movement vector (the actual units it will move)
			glm::vec3 movement = (t*speed) * (glm::normalize(toTarget_flat));

			parent_->Translate(movement);
		}

		if (glm::length(toTarget_flat) > 1) {
			// rotate on the y axis (turn towards where the target is in 2D space)
			// make a triangle from target and self pos, take angle, rotate
			float angle = asin(toTarget.x / sqrt(pow(toTarget.x, 2) + pow(toTarget.z, 2)));
			if (toTarget.z < 0)
			{
				angle = glm::pi<float>() - angle;
			}

			glm::quat hor_rotation = glm::angleAxis(angle, glm::vec3(0.0, 1.0, 0.0));

			glm::quat rotation = glm::slerp(GetOrientation(), hor_rotation, 0.05f);
			SetOrientation(rotation);
		}
	}

}