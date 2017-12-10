#include "bomb.h"
#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>

namespace game {
	Bomb::Bomb(const std::string name, const Resource *geometry, const Resource *material,
		double ttl, glm::vec3 rgb, const Resource *tex) : SceneNode(name, geometry, material, tex)
	{
		time_to_live = ttl;
		rgb_col = rgb;
	}

	Bomb::~Bomb() {}

	glm::vec3 Bomb::GetDir(void) const {
		return direction;
	}

	void Bomb::SetDir(glm::vec3 dir) {
		direction = dir;
	}

	void Bomb::Update(double delta_time) {
		time_to_live -= delta_time;
		if (time_to_live <= 0.0 && time_to_live > -4000.0) {
			destroyed = true;
		}
	}

	glm::mat4 Bomb::Draw(Camera *camera, glm::mat4 parent_transf, bool sun) {

		parent_transf = glm::mat4(1.0);

		// Disable z-buffer
		glDisable(GL_DEPTH_TEST);

		// Enable blending
		glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Simpler form
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);


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

	glm::mat4 Bomb::SetupShader(GLuint program, glm::mat4 parent_transf, bool sun) {
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
		glm::mat4 transf = translation * rotation * scaling;

		GLint world_mat = glGetUniformLocation(program, "world_mat");
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(transf));

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
		//double current_time = glfwGetTime();
		double current_time = (time_to_live - 4.0)*-1.0;
		glUniform1f(timer_var, (float)current_time);

		//color
		GLint red_var = glGetUniformLocation(program, "red");
		glUniform1f(red_var, (float)rgb_col[0]);

		GLint green_var = glGetUniformLocation(program, "green");
		glUniform1f(green_var, (float)rgb_col[1]);

		GLint blue_var = glGetUniformLocation(program, "blue");
		glUniform1f(blue_var, (float)rgb_col[2]);

		// Return transformation of node combined with parent, without scaling
		return transf;
	}

} // namespace game
