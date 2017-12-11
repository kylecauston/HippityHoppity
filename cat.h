#ifndef CAT_H_
#define CAT_H_
#include "enemy.h"

namespace game {
	class Cat : public Enemy {
	public:
		Cat(const std::string name, SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex = NULL);
		~Cat();

		void Update(double t);
		AttackNode* getAttack();

	private:
		static float damage;
	};
}
#endif // CAT_H_