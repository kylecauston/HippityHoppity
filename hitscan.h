#ifndef HITSCAN_H_
#define HITSCAN_H_
#include "attack_node.h"

namespace game {
	class Hitscan : public AttackNode {
		// class to represent hitscan attack styles
	public:
		Hitscan(Ray _r, float dam);
		Hitscan(glm::vec3 origin, glm::vec3 dir, float dam);

		Ray getRay(void) const;

	private:
		static int ID;
		Ray r;
	};
}
#endif  // HITSCAN_H