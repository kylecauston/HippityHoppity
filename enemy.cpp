#include "enemy.h"

namespace game {

	Enemy::Enemy(const std::string name, const SceneNode* targ, const Resource *geometry, const Resource *material, const Resource *tex) : SceneNode(name, geometry, material, tex) {
		target = targ;
	}

	Enemy::~Enemy() {
	}

	void Enemy::Update(float deltaTime) {
		// get the plane vector towards target
		glm::vec3 toTarget = target->GetPosition() - GetPosition();
		// 2D to target vector, ignoring height
		glm::vec3 toTarget_flat = glm::vec3(toTarget.x, 0, toTarget.z);

		if (glm::length(toTarget_flat) > deltaTime*speed) {

			// we drop the y value if the enemy can't fly, and then make 
			// the vector into a movement vector (the actual units it will move)
			glm::vec3 movement = (deltaTime*speed) * (glm::normalize(toTarget));
			
			Translate(movement);
		}

		toTarget = target->GetPosition() - GetPosition();
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
			float vert_angle = asin(toTarget.y / glm::length(glm::vec3(toTarget.x,
				toTarget.y,
				toTarget.z)));

			// find the axis to rotate on, by taking cross(forward, UP)
			glm::vec3 forward = hor_rotation * SceneNode::default_forward * -hor_rotation;
			glm::quat vert_rotation = glm::angleAxis(vert_angle, glm::cross(forward, glm::vec3(0.0, 1.0, 0.0)));
			glm::quat rotation = glm::slerp(GetOrientation(), hor_rotation * vert_rotation, rotateSpeed);
			SetOrientation(rotation);
		}

		// if in range, attack

	}

	void Enemy::Attack() {
		glm::quat aiming = GetOrientation();
	}

	//https://github.com/opengl-tutorials/ogl/tree/master/common
	glm::quat Enemy::RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
		start = glm::normalize(start);
		dest = glm::normalize(dest);

		float cosTheta = glm::dot(start, dest);

		glm::vec3 rotationAxis;
		rotationAxis = glm::cross(start, dest);

		float s = sqrt((1 + cosTheta) * 2);
		float invs = 1 / s;

		return glm::quat(
			s * 0.5f,
			rotationAxis.x * invs,
			rotationAxis.y * invs,
			rotationAxis.z * invs
		);

	}

} // namespace game
