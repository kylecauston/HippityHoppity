#include "attack_node.h"

namespace game {
	
	AttackNode::AttackNode(std::string s, const Resource* geom, const Resource* mat, const Resource* tex) 
		: SceneNode(s, geom, mat, tex, true) {

	}

	float AttackNode::getDamage(void) const {
		return damage;
	}

	void AttackNode::setDamage(float d) {
		damage = d;
	}
}