#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_
#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include "resource.h"
#include "camera.h"
#include "collidable.h"

namespace game {
	// Class that manages one object in a scene 
	class SceneNode : public Collidable {
	public:
		// Create scene node from given resources
		SceneNode(const std::string name, const Resource *geometry, const Resource *material, const Resource *tex = NULL, bool collision = false);

		// Destructor
		~SceneNode();

		// Get name of node
		const std::string GetName(void) const;
		std::string GetEntityName(void);

		// Get node attributes
		glm::vec3 GetPosition(void) const;
		glm::vec3 GetAbsolutePosition(void);
		glm::vec3 GetEntityPosition(void);
		glm::quat GetOrientation(void) const;
		glm::vec3 GetScale(void) const;
		float GetHealth(void) const;
		bool isCollidable(void) const;
		bool isDestroyed(void) const;

		// Set node attributes
		void SetPosition(glm::vec3 position);
		void SetOrientation(glm::quat orientation);
		void SetScale(glm::vec3 scale);
		void SetPosition(float x, float y, float z);
		void SetScale(float x, float y, float z);
		void setCollidable(bool c);
		void takeDamage(float d);

		// Perform transformations on node
		void Translate(glm::vec3 trans);
		void Rotate(glm::quat rot);
		void Scale(glm::vec3 scale);
		void Translate(float x, float y, float z);
		void Scale(float x, float y, float z);

		// Draw the node according to scene parameters in 'camera'
		virtual glm::mat4 Draw(Camera *camera, glm::mat4 parent_transf, bool sun);

		// Update the node
		virtual void Update(double deltaTime);
		virtual void onCollide(Collidable* other);

		// OpenGL variables
		GLenum GetMode(void) const;
		GLuint GetArrayBuffer(void) const;
		GLuint GetElementArrayBuffer(void) const;
		GLsizei GetSize(void) const;
		GLuint GetMaterial(void) const;

		// Hierarchy-related methods
		void AddChild(SceneNode *node);
		std::vector<SceneNode *>::const_iterator children_begin() const;
		std::vector<SceneNode *>::const_iterator children_end() const;
		std::vector<SceneNode *> children_;

		SceneNode *parent_;
		double time_to_live = -5000.0;
		bool destroyed = false;
		static glm::vec3 default_forward;

	protected:
		void destroy();
		glm::quat VectorToRotation(glm::vec3 v);

		std::string name_; // Name of the scene node
		GLuint array_buffer_; // References to geometry: vertex and array buffers
		GLuint element_array_buffer_;
		GLenum mode_; // Type of geometry
		GLsizei size_; // Number of primitives in geometry
		GLuint material_; // Reference to shader program
		GLuint texture_; // Reference to texture resource
		glm::vec3 position_; // Position of node
		glm::quat orientation_; // Orientation of node
		glm::vec3 scale_; // Scale of node

		float health = INFINITY;
		bool enemy = false;
		bool collidable = false;

		virtual glm::mat4 SetupShader(GLuint program, glm::mat4 parent_transf, bool sun);
	}; // class SceneNode
} // namespace game
#endif // SCENE_NODE_H_
