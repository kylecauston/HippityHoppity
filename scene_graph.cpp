#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stack>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene_graph.h"

namespace game {
	SceneGraph::SceneGraph(void) {
		background_color_ = glm::vec3(0.0, 0.0, 0.0);
	}

	SceneGraph::~SceneGraph() {}

	void SceneGraph::SetBackgroundColor(glm::vec3 color) {
		background_color_ = color;
	}

	glm::vec3 SceneGraph::GetBackgroundColor(void) const {
		return background_color_;
	}

	void SceneGraph::SetRoot(SceneNode *node) {
		root_ = node;
	}

	SceneNode *SceneGraph::GetNode(std::string node_name) const {
		// Find node with the specified name
		std::stack<SceneNode *> stck;
		stck.push(root_);
		while (stck.size() > 0) {
			SceneNode *current = stck.top();
			stck.pop();
			if (current->GetName() == node_name) {
				return current;
			}
			for (std::vector<SceneNode *>::const_iterator it = current->children_begin();
				it != current->children_end(); it++) {
				stck.push(*it);
			}
		}
		return NULL;
	}

	void SceneGraph::Draw(Camera *camera) {
		// Clear background
		glClearColor(background_color_[0],
			background_color_[1],
			background_color_[2], 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw all scene nodes
		// Initialize stack of nodes
		std::stack<SceneNode *> stck;
		stck.push(root_);
		// Initialize stack of transformations
		std::stack<glm::mat4> transf;
		transf.push(glm::mat4(1.0));
		// Traverse hierarchy
		while (stck.size() > 0) {
			// Get next node to be processed and pop it from the stack
			SceneNode *current = stck.top();
			stck.pop();
			// Get transformation corresponding to the parent of the next node
			glm::mat4 parent_transf = transf.top();
			transf.pop();
			// Draw node based on parent transformation
			glm::mat4 current_transf = current->Draw(camera, parent_transf);
			
			// if the current node has collision, update it's collidables
			if(current->isCollidable())
				current->updateCollidable(current_transf);

			// Push children of the node to the stack, along with the node's
			// transformation
			for (std::vector<SceneNode *>::const_iterator it = current->children_begin();
				it != current->children_end(); it++) {
				stck.push(*it);
				transf.push(current_transf);
			}
		}
	}

	void SceneGraph::Update(float deltaTime) {
		// Traverse hierarchy to update all nodes
		std::stack<SceneNode *> stck;
		stck.push(root_);
		while (stck.size() > 0) {
			SceneNode *current = stck.top();
			stck.pop();
			current->Update(deltaTime);
			for (std::vector<SceneNode *>::const_iterator it = current->children_begin();
				it != current->children_end(); it++) {
				stck.push(*it);
			}
		}
	}

	SceneNode* SceneGraph::FindName(std::string node_name) const { //just some cheese used for laser testing
		for (int i = 0; i < root_->children_.size(); i++) {
			if (root_->children_[i]->GetName() == node_name) {
				return root_->children_[i];
			}
		}
		return NULL;
	}

	void SceneGraph::Remove(std::string node_name) { //cheesily removes node of a given name
		std::vector<SceneNode*>::iterator position = std::find(root_->children_.begin(), root_->children_.end(), FindName(node_name));
		if (position != root_->children_.end()) {
			root_->children_.erase(position);
		}
	}

	void SceneGraph::CheckCollisions() {
		//std::cout << std::endl << "START" << std::endl;

		for (std::vector<SceneNode *>::const_iterator n1 = root_->children_begin();
			n1 != root_->children_end()-1; n1++) {
		
			// if this node isn't collidable, we don't need to check it's collisions
			if (!(*n1)->isCollidable())	continue;

			for (std::vector<SceneNode *>::const_iterator n2 = n1+1;
				n2 != root_->children_end(); n2++) {

				// if this node isn't collidable, don't check collisions
				if (!(*n2)->isCollidable()) continue;
				
				if (CollisionManager::checkHierarchicalCollision(*n1, *n2)) {
					std::cout << "Collision between " << (*n1)->GetName() << " and " << (*n2)->GetName() << std::endl;
				}
			}
		}
	}

} // namespace game