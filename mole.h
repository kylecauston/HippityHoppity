#ifndef MOLE_H_
#define MOLE_H_
#include "enemy.h"

namespace game {
	class Mole : public Enemy {
	public:
		Mole(const std::string name, SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex = NULL);
		~Mole();

		void Update(double t);
		AttackNode* getAttack();

	private:
		static float damage;
	};
}
#endif // MOLE_H_