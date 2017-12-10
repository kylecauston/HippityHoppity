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
			glm::mat4 current_transf = current->Draw(camera, parent_transf, true);

			// if the current node has collision, update it's collidables
			if (current->isCollidable())
				current->updateCollidable(current_transf);

			// Push children of the node to the stack
			for (std::vector<SceneNode *>::const_iterator it = current->children_begin();
				it != current->children_end(); it++) {
				stck.push(*it);
				transf.push(current_transf);
			}
		}
	}

	void SceneGraph::Update(double deltaTime) {
		// Traverse hierarchy to update all nodes
		std::stack<SceneNode *> stck;
		stck.push(root_);
		while (stck.size() > 0) {
			SceneNode *current = stck.top();
			stck.pop();

			if (current->isDestroyed()) {
				Remove(current->GetName());
				continue;
			}

			current->Update(deltaTime);

			// if we're updating an enemy, we wanna see if they can attack
			if (Enemy* e = dynamic_cast<Enemy*>(current)) {

				// if they are attacking, 
				if (e->isAttacking())
				{
					EnemyAttacking(e);
				}
			}

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

	void SceneGraph::Remove(std::string node_name) {
		// uses bfs to remove a node from the scenegraph
		// uses bfs because most of the time the removed node is a first child/entity

		std::queue<SceneNode*> q = std::queue<SceneNode*>();
		q.push(root_);

		SceneNode* n;
		while (!q.empty()) {
			n = q.front();
			q.pop();

			if (n->GetName() == node_name) {
				// if we found the node, remove it from the parent's list
				std::vector<SceneNode*>::iterator position = std::find(n->parent_->children_.begin(), n->parent_->children_.end(), n);
				if (position != n->parent_->children_.end()) {
					n->parent_->children_.erase(position);
					return;
				}
				else {
					std::cout << "this shouldn't happen" << std::endl;
				}
			}
			else {
				// if we didn't find the node, add this node's children to the queue
				for (SceneNode* c : n->children_) {
					q.push(c);
				}
			}
		}
	}

	// If an enemy has raised the attack flag, we handle the attack here
	void SceneGraph::EnemyAttacking(Enemy* e) {
		// grab their attack 
		AttackNode* a = e->getAttack();

		// if it's a hitscan attack, run the collision right now
		if (Hitscan* hs = dynamic_cast<Hitscan*>(a)) {
			// we get a list of <Node, Intersection Point> pairs back from CheckRayCollision
			std::vector <std::pair<SceneNode*, glm::vec2*>> list = CheckRayCollisions(hs->getRay());

			// cycle through the pairs and find the closest (non-current) node
			float min = INFINITY;
			SceneNode* closest = NULL;
			for (std::pair<SceneNode*, glm::vec2*> n : list) {
				// don't shoot ourselves
				if (e->GetName().find(n.first->GetName()) != std::string::npos) continue;

				// if this node is closer than the previous closest
				if (n.second->x < min) {
					min = n.second->x;
					closest = n.first;
				}
			}

			// now deal damage to the closest node, if there is one
			if (closest != NULL) {
				closest->takeDamage(a->getDamage());
				std::cout << closest->GetName() << ": " << closest->GetHealth() << "HP" << std::endl;
			}
		}
		else  // if it's not hitscan, we add it to the scene
		{
			AddProjectile(a);
		}
	}

	void SceneGraph::AddProjectile(SceneNode* p) {
		if (projectiles == NULL)
		{
			projectiles = new SceneNode("Proj_Dummy", NULL, NULL, NULL, false);
			projectiles->setCollidable(false);
			root_->AddChild(projectiles);
		}
		projectiles->AddChild(p);
	}

	/*   Cycle through each of the entities in the scene (first children of root).
	   Run collisions on each pair once, such that in nodes ABCD, comparisons
	   are AB AC AD BC BD CD. */
	void SceneGraph::CheckCollisions() {
		// start at the root and work through each node until second last
		for (std::vector<SceneNode *>::const_iterator n1 = root_->children_begin();
			n1 != root_->children_end() - 1; n1++) {

			if ((*n1) == projectiles) {
				continue;
			}

			// start at the node after n1 so we compare the next node (n2) to n1
			for (std::vector<SceneNode *>::const_iterator n2 = n1 + 1;
				n2 != root_->children_end(); n2++) {

				if ((*n2) == projectiles) {
					continue;
				}

				if (CollisionManager::checkHierarchicalCollision(*n1, *n2)) {
					//std::cout << "Collision between " << (*n1)->GetName() << " and " << (*n2)->GetName() << std::endl;
				}
			}
		}

		// now compare each entity to projectiles
		if (projectiles != NULL) {
			for (std::vector<SceneNode *>::const_iterator entity = root_->children_begin();
				entity != root_->children_end(); entity++) {

				if ((*entity) == projectiles) {
					continue;
				}

				for (std::vector<SceneNode *>::const_iterator p_n = projectiles->children_begin();
					p_n != projectiles->children_end(); p_n++) {

					Projectile* p = dynamic_cast<Projectile*>((*p_n));

					if (p->GetParentName() == (*entity)->GetName() || p->isDestroyed()) continue;

					if (CollisionManager::checkHierarchicalCollision(*entity, p)) {
						std::cout << "Proj Collision between " << (*entity)->GetName() << " and " << p->GetName() << std::endl;
						(*entity)->takeDamage(p->getDamage());
						p->takeDamage(INFINITY);
					}
				}
			}
		}
	}

	/* Cycle through all entities in scene (first children of root), and check against ray.
			Returns a list of pairs: <SceneNode*, Points of Intesection*>  */
	std::vector<std::pair<SceneNode*, glm::vec2*>> SceneGraph::CheckRayCollisions(Ray r) {
		std::vector<std::pair<SceneNode*, glm::vec2*>> hit_list = std::vector<std::pair<SceneNode*, glm::vec2*>>();

		glm::vec2* PoI;

		for (std::vector<SceneNode *>::const_iterator n = root_->children_begin();
			n != root_->children_end(); n++) {

			if (CollisionManager::checkHierarchicalCollision(*n, r, &PoI))
			{
				hit_list.push_back(std::pair<SceneNode*, glm::vec2*>((*n), PoI));
			}
		}

		return hit_list;
	}

	void SceneGraph::SetupDrawToTexture(void) {

		// Set up frame buffer
		glGenFramebuffers(1, &frame_buffer_);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);

		// Set up target texture for rendering
		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_2D, texture_);

		// Set up an image for the texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Set up a depth buffer for rendering
		glGenRenderbuffers(1, &depth_buffer_);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

		// Configure frame buffer (attach rendering buffers)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		// Check if frame buffer was setup successfully 
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw(std::ios_base::failure(std::string("Error setting up frame buffer")));
		}

		// Reset frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Set up quad for drawing to the screen
		static const GLfloat quad_vertex_data[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		};

		// Create buffer for quad
		glGenBuffers(1, &quad_array_buffer_);
		glBindBuffer(GL_ARRAY_BUFFER, quad_array_buffer_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_data), quad_vertex_data, GL_STATIC_DRAW);
	}

	void SceneGraph::DrawToTexture(Camera *camera, bool sun) {

		// Save current viewport
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		// Enable frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
		glViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

		// Clear background
		if (sun) {
			glClearColor(background_color_[0],
				background_color_[1],
				background_color_[2], 0.0);
		}
		else {
			glClearColor(background_color_[0] * 0.3,
				background_color_[1] * 0.4,
				background_color_[2] * 0.5, 0.0);
		}
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
			glm::mat4 current_transf = current->Draw(camera, parent_transf, sun);

			// if the current node has collision, update it's collidables
			if (current->isCollidable())
				current->updateCollidable(current_transf);

			// Push children of the node to the stack
			for (std::vector<SceneNode *>::const_iterator it = current->children_begin();
				it != current->children_end(); it++) {
				stck.push(*it);
				transf.push(current_transf);
			}
		}

		// Reset frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Restore viewport
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	}

	void SceneGraph::DisplayTexture(GLuint program, float hp) {

		// Configure output to the screen
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		// Set up quad geometry
		glBindBuffer(GL_ARRAY_BUFFER, quad_array_buffer_);

		// Select proper material (shader program)
		glUseProgram(program);

		// Setup attributes of screen-space shader
		GLint pos_att = glGetAttribLocation(program, "position");
		glEnableVertexAttribArray(pos_att);
		glVertexAttribPointer(pos_att, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

		GLint tex_att = glGetAttribLocation(program, "uv");
		glEnableVertexAttribArray(tex_att);
		glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));

		// hitpoints
		GLint hp_var = glGetUniformLocation(program, "hp");
		glUniform1f(hp_var, hp);

		// Bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_);

		// Draw geometry
		glDrawArrays(GL_TRIANGLES, 0, 6); // Quad: 6 coordinates

		// Reset current geometry
		glEnable(GL_DEPTH_TEST);
	}

} // namespace game