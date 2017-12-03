#include "collision_manager.h"
#include <iostream>

namespace game {
	bool CollisionManager::isColliding(Collidable* a, Collidable* b) {
		return (isColliding(a->aabb, b->aabb) && isColliding(a->hb, b->hb));
	}

	bool CollisionManager::isColliding(AABB a, AABB b)
	{
		// use simple geometry to detect collision between two AABB
		return (glm::abs(a.getPos().x - b.getPos().x) * 2 < a.getScale().x + b.getScale().x)
			&& (glm::abs(a.getPos().y - b.getPos().y) * 2 < a.getScale().y + b.getScale().y)
			&& (glm::abs(a.getPos().z - b.getPos().z) * 2 < a.getScale().z + b.getScale().z);
	}

	bool CollisionManager::isColliding(Hitbox a, Hitbox b)
	{
		// use SAT to detect collisions between two OBB

		glm::vec3 aPos = a.getPos();
		glm::vec3 bPos = b.getPos();
		glm::vec3 aScale = a.getScale();
		glm::vec3 bScale = b.getScale();
		glm::vec3 aHalfScale = glm::vec3(aScale.x / 2.0, aScale.y / 2.0, aScale.z / 2.0);
		glm::vec3 bHalfScale = glm::vec3(bScale.x / 2.0, bScale.y / 2.0, bScale.z / 2.0);
		
		// use the axis defined by the edges of the two cubes
		glm::vec3 aX, aY, aZ, bX, bY, bZ;
		aX = rotateAxis(glm::vec3(1.0, 0.0, 0.0), a.getTrans());
		aX = aX / glm::length(aX);
		aY = rotateAxis(glm::vec3(0.0, 1.0, 0.0), a.getTrans());
		aY = aY / glm::length(aY);
		aZ = rotateAxis(glm::vec3(0.0, 0.0, 1.0), a.getTrans());
		aZ = aZ / glm::length(aZ);

		bX = rotateAxis(glm::vec3(1.0, 0.0, 0.0), b.getTrans());
		bX = bX / glm::length(bX);
		bY = rotateAxis(glm::vec3(0.0, 1.0, 0.0), b.getTrans());
		bY = bY / glm::length(bY);
		bZ = rotateAxis(glm::vec3(0.0, 0.0, 1.0), b.getTrans());
		bZ = bZ / glm::length(bZ);

		std::vector<glm::vec3> axis = std::vector<glm::vec3>();

		axis.push_back(aX);
		axis.push_back(aY);
		axis.push_back(aZ);
		axis.push_back(bX);
		axis.push_back(bY);
		axis.push_back(bZ);

		axis.push_back(glm::cross(aX, bX));
		axis.push_back(glm::cross(aX, bY));
		axis.push_back(glm::cross(aX, bZ));
		axis.push_back(glm::cross(aY, bX));
		axis.push_back(glm::cross(aY, bY));
		axis.push_back(glm::cross(aY, bZ));
		axis.push_back(glm::cross(aZ, bX));
		axis.push_back(glm::cross(aZ, bY));
		axis.push_back(glm::cross(aZ, bZ));

		// if even one axis is separating, then there's no collision
		for (glm::vec3 ax : axis) {
			if (glm::abs(glm::dot((b.getPos() - a.getPos()), ax)) 
				>
				glm::abs(glm::dot((aHalfScale.x * aX), ax)) +
				glm::abs(glm::dot((aHalfScale.y * aY), ax)) +
				glm::abs(glm::dot((aHalfScale.z * aZ), ax)) +
				glm::abs(glm::dot((bHalfScale.x * bX), ax)) +
				glm::abs(glm::dot((bHalfScale.y * bY), ax)) +
				glm::abs(glm::dot((bHalfScale.z * bZ), ax))) {

				return false;
			}
		}

		return true;
	}

	bool CollisionManager::checkHierarchicalCollision(SceneNode* a, SceneNode* b) {
		// list of all nodes in trees
		std::vector<SceneNode*> a_list = std::vector<SceneNode*>();
		std::vector<SceneNode*> b_list = std::vector<SceneNode*>();
		
		int i = 0;
		SceneNode* curNode = a;
		
		std::vector<SceneNode*> children;

		a_list.push_back(curNode);
		//std::cout << "Creating Treelists a" << std::endl;
		while (i < a_list.size()) {
			curNode = a_list[i];

			children = curNode->children_;
			for (SceneNode* c : children)
			{
				//std::cout << "Adding node " << c->GetName() << " to a_list." << std::endl;
				a_list.push_back(c);
			}
			//std::cout << "Done adding " << curNode->GetName() << "'s children." << std::endl;

			i++;
		}

		i = 0;
		curNode = b;
		b_list.push_back(curNode);
		//std::cout << "Creating Treelist b" << std::endl;
		while (i < b_list.size()) {
			curNode = b_list[i];

			children = curNode->children_;
			for (SceneNode* c : children)
			{
				//std::cout << "Adding node " << c->GetName() << " to a_list." << std::endl;
				b_list.push_back(c);
			}
			//std::cout << "Done adding " << curNode->GetName() << "'s children." << std::endl;

			i++;
		}

		for (SceneNode* n1 : a_list)
		{
			if (!n1->isCollidable()) continue;
			for (SceneNode* n2 : b_list)
			{
				if (!n2->isCollidable()) continue;

				if (isColliding(n1, n2)) return true;
			}
		}

		return false;
	}

	glm::vec3 CollisionManager::rotateAxis(glm::vec3 v, glm::mat4 t) {
		glm::vec4 w_point = glm::vec4(v, 0.0);
		w_point = t * w_point;

		return glm::vec3(w_point.x, w_point.y, w_point.z);
	}
}