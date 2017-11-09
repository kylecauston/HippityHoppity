#include "laser.h"

namespace game {

Laser::Laser(const std::string name, const Resource *geometry, const Resource *material) : SceneNode(name, geometry, material) {
}


Laser::~Laser(){
}


glm::vec3 Laser::GetDir(void) const {

    return direction;
}


void Laser::SetDir(glm::vec3 dir){

    direction = dir;
}


void Laser::Update(void){

	glm::quat rotation = glm::angleAxis(glm::pi<float>() / 180.0f, glm::vec3(0.0,0.0,1.0));
	this->SetOrientation(this->GetOrientation() * rotation); //laser cube spins as it flies away to give more of a "lasery" impression

	this->Translate(direction); //the shot keeps moving in the direction we originally fired it from
}
            
} // namespace game
