#include "collidable.h"
#include <iostream>

namespace game {

	void Collidable::updateCollidable(glm::mat4 trans)
	{
		hb.setTransform(trans);
		std::vector<glm::vec3> points = hb.getPoints();

		// Now create the AABB for the hitbox
		// We accomplish this by taking the extremes of the hitbox and making a box around it

		// right/left = +/- x
		// up/down = +/- y 
		// close/far = +/- z
		float right = NAN, left = NAN, up = NAN, down = NAN, close = NAN, far = NAN;

		for (glm::vec3 p : points)
		{
			right = fmax(right, p.x);
			left = fmin(left, p.x);

			up = fmax(up, p.y);
			down = fmin(down, p.y);

			close = fmax(close, p.z);
			far = fmin(far, p.z);
		}

		glm::vec3 pos = glm::vec3((right + left) / 2.0, (up + down) / 2.0, (close + far) / 2.0);
		glm::vec3 scale = glm::vec3(right - left, up - down, close - far);

		aabb = AABB(pos, scale);
	}
}