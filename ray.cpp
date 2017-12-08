#include "ray.h"
#include<iostream>

namespace game {
	Ray::Ray() {
		// this is specifically not available but it won't stop complaining that I need it
	}

	Ray::Ray(glm::vec3 o, glm::vec3 d) {
		origin = o;
		direction = d / glm::length(d);
	}

	glm::vec3 Ray::getOrigin() { return origin; }

	glm::vec3 Ray::getDirection() { return direction; }

	glm::vec3 Ray::getPoint(float t) { return origin + direction*t; }
}