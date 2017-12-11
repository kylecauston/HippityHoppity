#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>
#include "scene_node.h"

namespace game {
	glm::vec3 SceneNode::default_forward = glm::vec3(0.0, 0.0, 1.0);

	SceneNode::SceneNode(const std::string name, const Resource *geometry, const Resource *material, const Resource *tex, bool collision) {
		// Set name of scene node
		name_ = name;

		if (geometry) {
			// Set geometry
			if (geometry->GetType() == PointSet) {
				mode_ = GL_POINTS;
			}
			else if (geometry->GetType() == Mesh) {
				mode_ = GL_TRIANGLES;
			}
			else {
				throw(std::invalid_argument(std::string("Invalid type of geometry")));
			}
			array_buffer_ = geometry->GetArrayBuffer();
			element_array_buffer_ = geometry->GetElementArrayBuffer();
			size_ = geometry->GetSize();

			hb = geometry->GetHitbox();
		}
		else {
			array_buffer_ = 0;
		}

		// Set material (shader program)
		if (material) {
			if (material->GetType() != Material) {
				throw(std::invalid_argument(std::string("Invalid type of material")));
			}
			material_ = material->GetResource();
		}
		else {
			material_ = 0;
		}

		if (tex) {
			texture_ = tex->GetResource();
		}
		else {
			texture_ = 0;
		}

		// Other attributes
		scale_ = glm::vec3(1.0, 1.0, 1.0);

		// Hierarchy
		parent_ = NULL;

		collidable = collision;
	}

	SceneNode::~SceneNode() {}

	const std::string SceneNode::GetName(void) const {
		return name_;
	}

	std::string SceneNode::GetEntityName(void) {
		SceneNode* parent = this->parent_;
		SceneNode* last = this;

		if (parent == NULL)
			return parent->GetName();

		// go until we've reached the root
		while (parent->parent_ != NULL)
		{
			last = parent;
			parent = parent->parent_;

		}

		return last->GetName();
	}

	glm::vec3 SceneNode::GetPosition(void) const {
		return position_;
	}

	/* Get the worldspace coords. */
	glm::vec3 SceneNode::GetAbsolutePosition(void) {
		SceneNode* n = this;
		glm::vec3 v = glm::vec3(0, 0, 0);

		// go until we've reached the root
		while (n != NULL)
		{
			v += n->GetPosition();
			n = n->parent_;
		}

		return v;
	}

	/* Get the Entity coords (where on the map they are). */
	glm::vec3 SceneNode::GetEntityPosition(void) {
		SceneNode* n = this;
		glm::vec3 v = glm::vec3(0, 0, 0);

		// go until we've reached the root, but don't add root
		while (n->parent_ != NULL)
		{
			v += n->GetPosition();
			n = n->parent_;
		}

		return v;
	}

	glm::quat SceneNode::GetOrientation(void) const {
		return orientation_;
	}

	glm::vec3 SceneNode::GetScale(void) const {
		return scale_;
	}

	float SceneNode::GetHealth(void) const { return health; }

	bool SceneNode::isCollidable(void) const {
		return collidable;
	}

	bool SceneNode::isDestroyed(void) const {
		return destroyed;
	}

	void SceneNode::SetPosition(glm::vec3 position) {
		position_ = position;
	}

	void SceneNode::SetPosition(float x, float y, float z) {
		SetPosition(glm::vec3(x, y, z));
	}

	void SceneNode::SetOrientation(glm::quat orientation) {
		orientation_ = orientation;
	}

	void SceneNode::SetScale(glm::vec3 scale) {
		scale_ = scale;
		hb.setScale(scale);
	}

	void SceneNode::SetScale(float x, float y, float z) {
		SetScale(glm::vec3(x, y, z));
	}

	void SceneNode::setCollidable(bool c) {
		collidable = c;
	}

	void SceneNode::takeDamage(float dam) {
		health -= dam;
		if (health <= 0)
		{
			health = 0;
			destroy();
		}
	}

	void SceneNode::destroy()
	{
		destroyed = true;
	}

	void SceneNode::Translate(glm::vec3 trans) {
		position_ += trans;
	}

	void SceneNode::Translate(float x, float y, float z) {
		Translate(glm::vec3(x, y, z));
	}

	void SceneNode::Rotate(glm::quat rot) {
		orientation_ *= rot;
	}

	void SceneNode::Scale(glm::vec3 scale) {
		SetScale(scale_ * scale);
	}

	void SceneNode::Scale(float x, float y, float z) {
		Scale(glm::vec3(x, y, z));
	}

	GLenum SceneNode::GetMode(void) const {
		return mode_;
	}

	GLuint SceneNode::GetArrayBuffer(void) const {
		return array_buffer_;
	}

	GLuint SceneNode::GetElementArrayBuffer(void) const {
		return element_array_buffer_;
	}

	GLsizei SceneNode::GetSize(void) const {
		return size_;
	}

	GLuint SceneNode::GetMaterial(void) const {
		return material_;
	}

	glm::mat4 SceneNode::Draw(Camera *camera, glm::mat4 parent_transf, bool sun) {
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		if (name_ == "Laser1") { //laser drawn in front of the helicopter for now
			parent_transf = glm::mat4(1.0);
		}

		if ((array_buffer_ > 0) && (material_ > 0)) {
			// Select proper material (shader program)
			glUseProgram(material_);

			// Set geometry to draw
			glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_);

			// Set globals for camera
			camera->SetupShader(material_);

			// Set world matrix and other shader input variables
			glm::mat4 transf = SetupShader(material_, parent_transf, sun);

			// Draw geometry
			if (mode_ == GL_POINTS) {
				glDrawArrays(mode_, 0, size_);
			}
			else {
				glDrawElements(mode_, size_, GL_UNSIGNED_INT, 0);
			}

			return transf;
		}
		else {
			glm::mat4 rotation = glm::mat4_cast(orientation_);
			glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);
			glm::mat4 transf = parent_transf * translation * rotation;
			return transf;
		}
	}

	void SceneNode::Update(double deltaTime) {
		// Do nothing for this generic type of scene node
	}

	void SceneNode::onCollide(Collidable* other) {
		takeDamage(0.1);

		// if this collided with the world bounds, destroy object
		if (SceneNode* n = dynamic_cast<SceneNode*>(other))
		{
			if (n->GetName() == "Ground_Box")
			{
				destroy();
			}
		}
	}

	glm::quat SceneNode::VectorToRotation(glm::vec3 v) {
		// get the rotation around the y axis
		float angle = asin(v.x / sqrt(pow(v.x, 2) + pow(v.z, 2)));

		if (v.z < 0)
			angle = glm::pi<float>() - angle;

		glm::quat hor_rotation = glm::angleAxis(angle, glm::vec3(0.0, 1.0, 0.0));

		// get the vertical angle (no more than 90deg)
		float vert_angle = asin(v.y / glm::length(v));

		glm::quat vert_rotation = glm::angleAxis(vert_angle, glm::cross(SceneNode::default_forward, glm::vec3(0.0, 1.0, 0.0)));

		return hor_rotation * vert_rotation;
	}

	glm::mat4 SceneNode::SetupShader(GLuint program, glm::mat4 parent_transf, bool sun) {
		// Set attributes for shaders
		GLint vertex_att = glGetAttribLocation(program, "vertex");
		glVertexAttribPointer(vertex_att, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(vertex_att);

		GLint normal_att = glGetAttribLocation(program, "normal");
		glVertexAttribPointer(normal_att, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normal_att);

		GLint color_att = glGetAttribLocation(program, "color");
		glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(color_att);

		GLint tex_att = glGetAttribLocation(program, "uv");
		glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void *)(9 * sizeof(GLfloat)));
		glEnableVertexAttribArray(tex_att);

		// World transformation
		glm::mat4 scaling = glm::scale(glm::mat4(1.0), scale_);
		glm::mat4 rotation = glm::mat4_cast(orientation_);
		glm::mat4 translation = glm::translate(glm::mat4(1.0), position_);
		glm::mat4 transf = parent_transf * translation * rotation;
		glm::mat4 local_transf = transf * scaling;

		GLint world_mat = glGetUniformLocation(program, "world_mat");
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local_transf));

		// Normal matrix
		glm::mat4 normal_matrix = glm::transpose(glm::inverse(transf));
		GLint normal_mat = glGetUniformLocation(program, "normal_mat");
		glUniformMatrix4fv(normal_mat, 1, GL_FALSE, glm::value_ptr(normal_matrix));

		// Texture
		if (texture_) {
			GLint tex = glGetUniformLocation(program, "texture_map");
			glUniform1i(tex, 0); // Assign the first texture to the map
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture_); // First texture we bind
													// Define texture interpolation
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		// Timer
		GLint timer_var = glGetUniformLocation(program, "timer");
		double current_time = glfwGetTime();
		glUniform1f(timer_var, (float)current_time);

		// Light
		GLint light_var = glGetUniformLocation(program, "light");
		double light;
		if (sun) { light = 0.9; }
		else { light = 0.0; }
		glUniform1f(light_var, (float)light);

		// Return transformation of node combined with parent, without scaling
		return transf;
	}

	void SceneNode::AddChild(SceneNode *node) {
		children_.push_back(node);
		node->parent_ = this;
	}

	std::vector<SceneNode *>::const_iterator SceneNode::children_begin() const {
		return children_.begin();
	}

	std::vector<SceneNode *>::const_iterator SceneNode::children_end() const {
		return children_.end();
	}

} // namespace game;
