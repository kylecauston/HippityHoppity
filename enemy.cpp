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

		if (glm::length(glm::vec3(toTarget.x, 0, toTarget.z)) > deltaTime*speed) {

			// we drop the y value if the enemy can't fly, and then make 
			// the vector into a movement vector (the actual units it will move)
			glm::vec3 movement = (deltaTime*speed) * (glm::normalize(glm::vec3(toTarget.x, 0, toTarget.z)));
			
			//Translate(movement);
		}


		// rotate in a similar fashion
		// use Spherical Linear intERPolation to slowly rotate towards target
		// for this we need:

		//	Current Orientation - where are we currently looking 
		glm::quat curOrientation = GetOrientation();

		//	Desired Orientation - where do we want to be looking
		//		This is the orientation we have if we're looking at the target.
		glm::vec3 inFront = curOrientation*glm::vec3(0,1,0)*-curOrientation;
		glm::quat desiredOrientation = RotationBetweenVectors(inFront, toTarget);
		glm::quat rot = glm::slerp(curOrientation, desiredOrientation, rotateSpeed);
		
		this->SetOrientation(rot);
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
