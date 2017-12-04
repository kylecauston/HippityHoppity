#ifndef SCENE_GRAPH_H_
#define SCENE_GRAPH_H_
#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "scene_node.h"
#include "resource.h"
#include "camera.h"
#include "collision_manager.h"

namespace game {
	// Class that manages all the objects in a scene
	class SceneGraph {
	private:
		// Background color
		glm::vec3 background_color_;

	public:
		// Root of the hierarchy
		SceneNode * root_;

		SceneGraph(void);
		~SceneGraph();

		// Background color
		void SetBackgroundColor(glm::vec3 color);
		glm::vec3 GetBackgroundColor(void) const;

		// Set root of the hierarchy
		void SetRoot(SceneNode *node);
		// Find a scene node with a specific name
		SceneNode *GetNode(std::string node_name) const;
		SceneNode *FindName(std::string node_name) const;

		// Draw the entire scene
		void Draw(Camera *camera);

		// Update entire scene
		void Update(float deltaTime);

		// run collisions on the children of node (the separate entities)
		void CheckCollisions();
		std::vector<std::string> CheckRayCollisions(Ray r);

		void Remove(std::string node_name); //remove a node with a given name

	}; // class SceneGraph

} // namespace game

#endif // SCENE_GRAPH_H_
