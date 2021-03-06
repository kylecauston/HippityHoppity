#ifndef ATTACK_NODE_H_
#define ATTACK_NODE_H_
#include "scene_node.h"

namespace game {
	class AttackNode : public SceneNode {
		// class to represent attacks (mostly projectiles)
	public:
		void setDamage(float d);
		float getDamage(void) const;
	protected:
		AttackNode(std::string s, float dam, const Resource* geom, const Resource* material, const Resource* tex = NULL);
		float damage = 0.5;
	};
}
#endif  // ATTACK_NODE_H