#include "collision_manager.h"
#include <iostream>
#include <string>

namespace game {
	/* Return whether two collidable objects intersect. */
	bool CollisionManager::isColliding(Collidable* a, Collidable* b) {
		// check if the AABB's collide, and if they do, then check HB collision
		return (isColliding(a->aabb, b->aabb) && isColliding(a->hb, b->hb));
	}

	/* Return whether a collidable object is intersected by a ray. */
	bool CollisionManager::isColliding(Collidable* a, Ray r) {
		if (isColliding(a->aabb, r))
		{
			glm::vec2* PoI = NULL;
			if (isColliding(a->hb, r, &PoI))
			{
				std::cout << "[" << PoI->x << ", " << PoI->y << "]" << std::endl;
				return true;

			}
			
			return false;
			
		}

		return false;
	}

	/* Return whether two axis-aligned bounding boxes intersect. */
	bool CollisionManager::isColliding(AABB a, AABB b)
	{
		// use simple geometry to detect collision between two AABB
		return (glm::abs(a.getPos().x - b.getPos().x) * 2 < a.getScale().x + b.getScale().x)
			&& (glm::abs(a.getPos().y - b.getPos().y) * 2 < a.getScale().y + b.getScale().y)
			&& (glm::abs(a.getPos().z - b.getPos().z) * 2 < a.getScale().z + b.getScale().z);
	}

	/* Return whether two oriented bounding boxes (hitboxes) intersect. */
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
		aY = rotateAxis(glm::vec3(0.0, 1.0, 0.0), a.getTrans());
		aZ = rotateAxis(glm::vec3(0.0, 0.0, 1.0), a.getTrans());

		bX = rotateAxis(glm::vec3(1.0, 0.0, 0.0), b.getTrans());
		bY = rotateAxis(glm::vec3(0.0, 1.0, 0.0), b.getTrans());
		bZ = rotateAxis(glm::vec3(0.0, 0.0, 1.0), b.getTrans());

		// all axis to check for separation
		std::vector<glm::vec3> axis = std::vector<glm::vec3>();

		// edge axis
		axis.push_back(aX);
		axis.push_back(aY);
		axis.push_back(aZ);
		axis.push_back(bX);
		axis.push_back(bY);
		axis.push_back(bZ);

		// face axis (planes of separation)
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
			// to define separating axis: 
			// check if the projection of the distance between the two cubes on to the axis
			// is greater than
			// the sum of the projections of the half cube onto the axis
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

	/* Return whether a ray intersects an axis-aligned bounding box. */
	bool CollisionManager::isColliding(AABB a, Ray r) {
		glm::vec3 a_max = a.getPos() + a.getScale() / 2.0f;
		glm::vec3 a_min = a.getPos() - a.getScale() / 2.0f;

		float curMax, curMin;

		// calculate where the line intersects with x axis
		curMax = (a_max.x - r.getOrigin().x) / r.getDirection().x;
		curMin = (a_min.x - r.getOrigin().x) / r.getDirection().x;

		float temp;
		if (curMax < curMin) {
			temp = curMax;
			curMax = curMin;
			curMin = temp;
		}

		float yMax = (a_max.y - r.getOrigin().y) / r.getDirection().y;
		float yMin = (a_min.y - r.getOrigin().y) / r.getDirection().y;

		if (yMax < yMin) {
			temp = yMax;
			yMax = yMin;
			yMin = temp;
		}

		// check if the intersection misses on the y axis
		if ((curMin > yMax) || (curMax < yMin)) 
			return false;
		
		curMax = glm::min(curMax, yMax);
		curMin = glm::max(curMin, yMin);

		float zMax = (a_max.z - r.getOrigin().z) / r.getDirection().z;
		float zMin = (a_min.z - r.getOrigin().z) / r.getDirection().z;

		if (zMax < zMin) {
			temp = zMax;
			zMax = zMin;
			zMin = temp;
		}

		// check if the intersection misses on the z axis
		if ((curMin > zMax) || (curMax < zMin))
			return false;

		curMax = glm::min(curMax, zMax);
		curMin = glm::max(curMin, zMin);

		return true;
	}

	/*  Input: Hitbox a: hitbox of object
		       Ray r:	 ray to intersect
		Output: Intersection Points: solutions to the function made by ray/hb intersection.
	*/
	bool CollisionManager::isColliding(Hitbox a, Ray r, glm::vec2** intersection) {
		// max = nearest far of all intersections 
		// min = farthest near of ''
		float max = NAN, min = NAN;

		glm::vec3 to_targ = a.getPos() - r.getOrigin();

		// grab each axis
		glm::vec3 xa = rotateAxis(glm::vec3(1.0, 0.0, 0.0), a.getTrans());
		glm::vec3 ya = rotateAxis(glm::vec3(0.0, 1.0, 0.0), a.getTrans());
		glm::vec3 za = rotateAxis(glm::vec3(0.0, 0.0, 1.0), a.getTrans());

		float p1 = glm::dot(xa, to_targ);
		float p2 = glm::dot(r.getDirection(), xa);

		float dist1 = (p1 + a.getMinPoint().x) / p2;
		float dist2 = (p1 + a.getMaxPoint().x) / p2;

		// if they're in the wrong order, swap
		float temp;
		if (dist1 > dist2) {
			temp = dist1;
			dist1 = dist2;
			dist2 = temp;
		}

		min = glm::max(min, dist1);
		max = glm::min(max, dist2);

		if (max < min) 
			return false;



		p1 = glm::dot(ya, to_targ);
		p2 = glm::dot(r.getDirection(), ya);

		dist1 = (p1 + a.getMinPoint().y) / p2;
		dist2 = (p1 + a.getMaxPoint().y) / p2;

		// if they're in the wrong order, swap
		temp;
		if (dist1 > dist2) {
			temp = dist1;
			dist1 = dist2;
			dist2 = temp;
		}

		min = glm::max(min, dist1);
		max = glm::min(max, dist2);

		if (max < min)
			return false;



		p1 = glm::dot(za, to_targ);
		p2 = glm::dot(r.getDirection(), za);

		dist1 = (p1 + a.getMinPoint().z) / p2;
		dist2 = (p1 + a.getMaxPoint().z) / p2;

		// if they're in the wrong order, swap
		temp;
		if (dist1 > dist2) {
			temp = dist1;
			dist1 = dist2;
			dist2 = temp;
		}

		min = glm::max(min, dist1);
		max = glm::min(max, dist2);

		if (max < min)
			return false;

		(*intersection) = new glm::vec2(min, max);

		return true;
	}

	/* Take two hierarchical SceneNodes and check if either tree collides with the other. */
	bool CollisionManager::checkHierarchicalCollision(SceneNode* a, SceneNode* b) {
		// list of all nodes in trees
		std::vector<SceneNode*> a_list = flattenTree(a);
		std::vector<SceneNode*> b_list = flattenTree(b);

		for (SceneNode* n1 : a_list)
		{
			// ignore non-collidables
			if (!n1->isCollidable()) continue;
			for (SceneNode* n2 : b_list)
			{
				// ignore non-collidables
				if (!n2->isCollidable()) continue;

				if (isColliding(n1, n2)) return true;
			}
		}

		return false;
	}

	/* Take a single hierarchical SceneNode root and return if there is an intersection with Ray r. */
	bool CollisionManager::checkHierarchicalCollision(SceneNode* root, Ray r) {
		std::vector<SceneNode*> list = flattenTree(root);

		for (SceneNode* n : list)
		{

			// ignore non-collidables
			if (!n->isCollidable()) continue;

			if (isColliding(n, r)) return true;
		}

		return false;
	}

	/* Rotate the given vector by the transformation matrix. *Ignores translation* */
	glm::vec3 CollisionManager::rotateAxis(glm::vec3 v, glm::mat4 t) {
		glm::vec4 w_point = glm::vec4(v, 0.0);
		w_point = t * w_point;

		return glm::vec3(w_point.x, w_point.y, w_point.z);
	}

	/* Take a hierarchical SceneNode tree root and turn it into a 1D array. */
	std::vector<SceneNode*> CollisionManager::flattenTree(SceneNode* root)
	{
		std::vector<SceneNode*> list = std::vector<SceneNode*>();

		// where in the array we are
		int i = 0;
		SceneNode* curNode = root;

		std::vector<SceneNode*> children;

		list.push_back(curNode);
		//std::cout << "Creating Treelists a" << std::endl;
		while (i < list.size()) {
			curNode = list[i];

			children = curNode->children_;
			for (SceneNode* c : children)
			{
				//std::cout << "Adding node " << c->GetName() << " to a_list." << std::endl;
				list.push_back(c);
			}
			//std::cout << "Done adding " << curNode->GetName() << "'s children." << std::endl;

			i++;
		}

		return list;
	}
} // namespce game