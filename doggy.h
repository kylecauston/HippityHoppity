#ifndef DOGGY_H_
#define DOGGY_H_
#include "enemy.h"

namespace game {
	class Doggy : public Enemy {
	public:
		Doggy(const std::string name, const SceneNode* targ, const Resource* geometry, const Resource* mat, const Resource *tex = NULL);
		~Doggy();
		
		void Update(float t);
		void setTurret(SceneNode* t);

	private:
		SceneNode* turret;
	};
}


#endif // DOGGY_H_