#include "hitbox.h"

Hitbox::Hitbox(glm::vec3* p, glm::vec3* s, glm::quat* r) {
	position = p;
	scale = s;
	rotation = r;
}

Hitbox::~Hitbox() {}

std::vector<glm::vec3> Hitbox::getPoints() {
	std::vector<glm::vec3> points = std::vector<glm::vec3>();
	glm::vec3 v;
	for (float x = -1; x < 2; x++)
	{
		if (x == 0) continue;
		for (float y = -1; y < 2; y++)
		{
			if (y == 0) continue;
			for (float z = -1; z < 2; z++)
			{
				if (z == 0) continue;

				// first, create the unit vector pointing
				// from center to each point
				v = glm::vec3(x, y, z);
				v = glm::normalize(v);
				
				// then rotate the vector by the rotation
				v = *rotation * v * -*rotation;

				// scale it
				v *= *scale;

				// then translate it
				v += *position;

				points.push_back(glm::vec3(x, y, z));
			}
		}
	}
}

void Hitbox::setPosition(glm::vec3* p) {
	position = p;
}

void Hitbox::setScale(glm::vec3* s) {
	scale = s;
}

void Hitbox::setRotation(glm::quat* r) {
	rotation = r;
}
