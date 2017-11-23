#ifndef HITBOX_H_
#define HITBOX_H_
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

class Hitbox {
private:
	glm::vec3* position;
	glm::vec3* scale;
	glm::quat* rotation;

public:
	Hitbox(glm::vec3* p, glm::vec3* s, glm::quat* r);
	~Hitbox();

	// returns the 8 points made by this hitbox
	std::vector<glm::vec3> getPoints();

	void setPosition(glm::vec3* p);
	void setScale(glm::vec3* s);
	void setRotation(glm::quat* r);

};

#endif // HITBOX_H_