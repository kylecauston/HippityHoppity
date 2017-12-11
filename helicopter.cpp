#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "helicopter.h"

namespace game {
	Helicopter::Helicopter(void) {}

	Helicopter::~Helicopter() {}

	glm::vec3 Helicopter::GetPosition(void) const {
		return position_;
	}

	glm::quat Helicopter::GetOrientation(void) const {
		return orientation_;
	}

	void Helicopter::SetPosition(glm::vec3 position) {
		position_ = position;
	}

	void Helicopter::SetOrientation(glm::quat orientation) {
		orientation_ = orientation;
	}

	glm::vec3 Helicopter::GetForward(void) const {
		glm::vec3 current_forward = orientation_ * forward_;
		return -current_forward; // Return -forward since the camera coordinate system points in the opposite direction
	}

	glm::vec3 Helicopter::GetSide(void) const {
		glm::vec3 current_side = orientation_ * side_;
		return current_side;
	}

	glm::vec3 Helicopter::GetUp(void) const {
		glm::vec3 current_forward = orientation_ * forward_;
		glm::vec3 current_side = orientation_ * side_;
		glm::vec3 current_up = glm::cross(current_forward, current_side);
		current_up = glm::normalize(current_up);
		return current_up;
	}

	void Helicopter::Translate(glm::vec3 trans) {
		position_ += trans;
	}

	void Helicopter::Update(double timer) { //keep momentum going
		this->Translate(this->GetForward()*vel_z);
		this->Translate(this->GetSide()*vel_x);
		this->Translate(this->GetUp()*vel_y);

		// spin the helicopter's blades over time
		if (timer > 0.001) { // 1 miliseconds interval
			glm::quat topRotation = glm::angleAxis(2.0f * glm::pi<float>() / 180.0f, glm::vec3(0.0, 1.0, 0.0));
			glm::quat backRotation = glm::angleAxis(2.0f * glm::pi<float>() / 180.0f, glm::vec3(1.0, 0.0, 0.0));
			topOrientation *= topRotation;
			backOrientation *= backRotation;
		}
	}

	void Helicopter::SetView(glm::vec3 position, glm::vec3 look_at, glm::vec3 up) {
		// Store initial forward and side vectors
		// See slide in "Camera control" for details
		forward_ = look_at - position;
		forward_ = -glm::normalize(forward_);
		side_ = glm::cross(up, forward_);
		side_ = glm::normalize(side_);

		// Reset orientation and position of camera
		position_ = position;
		orientation_ = glm::quat();
	}

	void Helicopter::SetProjection(GLfloat fov, GLfloat Near, GLfloat Far, GLfloat w, GLfloat h) {
		// Set projection based on field-of-view
		float top = tan((fov / 2.0)*(glm::pi<float>() / 180.0))*Near;
		float right = top * w / h;
		projection_matrix_ = glm::frustum(-right, right, -top, top, Near, Far);
	}

	void Helicopter::SetupShader(GLuint program) {
		// Update view matrix
		SetupViewMatrix();
	}

	void Helicopter::SetupViewMatrix(void) {
		// Get current vectors of coordinate system
		// [side, up, forward]
		// See slide in "Camera control" for details
		glm::vec3 current_forward = orientation_ * forward_;
		glm::vec3 current_side = orientation_ * side_;
		glm::vec3 current_up = glm::cross(current_forward, current_side);
		current_up = glm::normalize(current_up);

		// Initialize the view matrix as an identity matrix
		view_matrix_ = glm::mat4(1.0);

		// Copy vectors to matrix
		// Add vectors to rows, not columns of the matrix, so that we get
		// the inverse transformation
		// Note that in glm, the reference for matrix entries is of the form
		// matrix[column][row]
		view_matrix_[0][0] = current_side[0]; // First row
		view_matrix_[1][0] = current_side[1];
		view_matrix_[2][0] = current_side[2];
		view_matrix_[0][1] = current_up[0]; // Second row
		view_matrix_[1][1] = current_up[1];
		view_matrix_[2][1] = current_up[2];
		view_matrix_[0][2] = current_forward[0]; // Third row
		view_matrix_[1][2] = current_forward[1];
		view_matrix_[2][2] = current_forward[2];

		// Create translation to camera position
		glm::mat4 trans = glm::translate(glm::mat4(1.0), -position_);

		// Combine translation and view matrix in proper order
		view_matrix_ *= trans;
	}

	int Helicopter::CreateCube(void) {
		// The construction does not use shared vertices, since we need to assign appropriate normals to each face 
		// Each face of the cube is defined by four vertices (with the same normal) and two triangles

		// Vertices used to build the cube
		// 9 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3)
		GLfloat vertex[] = {
			// First cube face (two triangles)
			-0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.4, 0.4, //modified these colors to create a nice shading
			0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.4, 0.4,
			0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.4, 0.4,
			0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.4, 0.4,
			-0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.4, 0.4,
			-0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.4, 0.4,
			// Second cube face
			0.5, -0.5,  0.5,    1.0,  0.0,  0.0,    0.45, 0.45, 0.45,
			0.5, -0.5, -0.5,    1.0,  0.0,  0.0,    0.45, 0.45, 0.45,
			0.5,  0.5, -0.5,    1.0,  0.0,  0.0,    0.45, 0.45, 0.45,
			0.5,  0.5, -0.5,    1.0,  0.0,  0.0,    0.45, 0.45, 0.45,
			0.5,  0.5,  0.5,    1.0,  0.0,  0.0,    0.45, 0.45, 0.45,
			0.5, -0.5,  0.5,    1.0,  0.0,  0.0,    0.45, 0.45, 0.45,
			// Third cube face
			0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.5, 0.5, 0.5,
			-0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.5, 0.5, 0.5,
			-0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.5, 0.5, 0.5,
			-0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.5, 0.5, 0.5,
			0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.5, 0.5, 0.5,
			0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.5, 0.5, 0.5,
			// Fourth cube face
			-0.5, -0.5, -0.5,   -1.0,  0.0,  0.0,    0.5, 0.5, 0.5,
			-0.5, -0.5,  0.5,   -1.0,  0.0,  0.0,    0.5, 0.5, 0.5,
			-0.5,  0.5,  0.5,   -1.0,  0.0,  0.0,    0.5, 0.5, 0.5,
			-0.5,  0.5,  0.5,   -1.0,  0.0,  0.0,    0.5, 0.5, 0.5,
			-0.5,  0.5, -0.5,   -1.0,  0.0,  0.0,    0.5, 0.5, 0.5,
			-0.5, -0.5, -0.5,   -1.0,  0.0,  0.0,    0.5, 0.5, 0.5,
			// Fifth cube face
			-0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.55, 0.55, 0.55,
			0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.55, 0.55, 0.55,
			0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    0.55, 0.55, 0.55,
			0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    0.55, 0.55, 0.55,
			-0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    0.55, 0.55, 0.55,
			-0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.55, 0.55, 0.55,
			// Sixth cube face
			0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    0.5, 0.5, 0.5,
			-0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    0.5, 0.5, 0.5,
			-0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    0.5, 0.5, 0.5,
			-0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    0.5, 0.5, 0.5,
			0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    0.5, 0.5, 0.5,
			0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    0.5, 0.5, 0.5,
		};
		// Create OpenGL buffer for vertices
		//GLuint qbo;
		glGenBuffers(1, &cubeVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

		// Return number of elements in array buffer
		return sizeof(vertex) / (sizeof(GLfloat) * 9);
	}

	int Helicopter::CreateCylinder(float cylinder_height, float circle_radius, int num_circle_samples) {
		// Number of vertices and faces to be created
		const GLuint vertex_num = num_circle_samples * 2 + 2; // both circles, center of each endcap
		const GLuint face_num = num_circle_samples * 4;

		// Number of attributes for vertices and faces
		const int vertex_att = 11;  // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
		const int face_att = 3; // Vertex indices (3)

		// Data buffers for the cylinder
		GLfloat *vertex = NULL;
		GLuint *face = NULL;

		// Allocate memory for buffers
		try {
			vertex = new GLfloat[vertex_num * vertex_att];
			face = new GLuint[face_num * face_att];
		}
		catch (std::exception &e) {
			throw e;
		}

		// Create vertices 
		float theta = 0; // Angle for circle
		glm::vec3 vertex_position;
		glm::vec3 vertex_normal;
		glm::vec3 vertex_color;
		glm::vec2 vertex_coord;

		// vertices at
		// - one in center of each endcap
		// - num_circle_samples worth on edges of each endcap

		// top centerpoint
		vertex_normal = glm::vec3(0, 0, 1);
		vertex_position = glm::vec3(0, cylinder_height / 2, 0);
		vertex_color = glm::vec3(1.0, 0.0, 0.0);
		vertex_coord = glm::vec2(theta / 2.0*glm::pi<GLfloat>(),
			theta / 2.0*glm::pi<GLfloat>());

		// Add vectors to the data buffer
		for (int k = 0; k < 3; k++) {
			vertex[k] = vertex_position[k];
			vertex[k + 3] = vertex_normal[k];
			vertex[k + 6] = vertex_color[k];
		}
		vertex[9] = vertex_coord[0];
		vertex[10] = vertex_coord[1];

		// bottom centerpoint
		vertex_normal = glm::vec3(0, 0, -1);
		vertex_position = glm::vec3(0, -cylinder_height / 2, 0);
		vertex_color = glm::vec3(1.0, 0.0, 0.0);

		// Add vectors to the data buffer
		for (int k = 0; k < 3; k++) {
			vertex[k + vertex_att] = vertex_position[k];
			vertex[k + 3 + vertex_att] = vertex_normal[k];
			vertex[k + 6 + vertex_att] = vertex_color[k];
		}
		vertex[9 + vertex_att] = vertex_coord[0];
		vertex[10 + vertex_att] = vertex_coord[1];

		for (int i = 0; i < num_circle_samples; i++) {
			theta = 2.0*glm::pi<GLfloat>() * i / num_circle_samples; // where around the endcap circle you are

			// top set of vertices
			vertex_normal = glm::vec3(0, 0, 1);
			vertex_position = glm::vec3((cos(theta)*circle_radius),
				cylinder_height / 2,
				(sin(theta)*circle_radius));
			vertex_color = glm::vec3(0.0, 0.0, 1.0);

			// Add vectors to the data buffer
			for (int k = 0; k < 3; k++) {
				vertex[(i + 2)*vertex_att + k] = vertex_position[k];
				vertex[(i + 2)*vertex_att + k + 3] = vertex_normal[k];
				vertex[(i + 2)*vertex_att + k + 6] = vertex_color[k];
			}
			vertex[(i + 2)*vertex_att + 9] = vertex_coord[0];
			vertex[(i + 2)*vertex_att + 10] = vertex_coord[1];

			// bottom set of vertices
			vertex_normal = glm::vec3(0, 0, -1);
			vertex_position = glm::vec3((cos(theta)*circle_radius),
				-cylinder_height / 2,
				(sin(theta)*circle_radius));
			vertex_color = glm::vec3(0.0, 0.0, 1.0);

			// Add vectors to the data buffer
			for (int k = 0; k < 3; k++) {
				vertex[(i + 2 + num_circle_samples)*vertex_att + k] = vertex_position[k];
				vertex[(i + 2 + num_circle_samples)*vertex_att + k + 3] = vertex_normal[k];
				vertex[(i + 2 + num_circle_samples)*vertex_att + k + 6] = vertex_color[k];
			}
			vertex[(i + 2 + num_circle_samples)*vertex_att + 9] = vertex_coord[0];
			vertex[(i + 2 + num_circle_samples)*vertex_att + 10] = vertex_coord[1];
		}

		// create triangles
		for (int i = 0; i < num_circle_samples - 1; i++) {
			// top cap triangle
			glm::vec3 t1(0, i + 2, i + 3);

			// middle triangle 1		
			glm::vec3 t2(i + 2, i + 3, i + 3 + num_circle_samples);

			// middle triangle 2
			glm::vec3 t3(i + 2, i + 2 + num_circle_samples, i + 3 + num_circle_samples);

			// bottom cap triangle
			glm::vec3 t4(1, i + 2 + num_circle_samples, i + 3 + num_circle_samples);

			for (int k = 0; k < 3; k++) {
				face[i * face_att * 4 + k] = (GLuint)t1[k];
				face[i * face_att * 4 + k + face_att] = (GLuint)t2[k];
				face[i * face_att * 4 + k + face_att * 2] = (GLuint)t3[k];
				face[i * face_att * 4 + k + face_att * 3] = (GLuint)t4[k];
			}
		}

		// add the remaining sides - missed by the loop
		// top cap triangle between start of circle and end
		glm::vec3 t1(0, 2, num_circle_samples + 1);

		// middle triangle 1		
		glm::vec3 t2(2, num_circle_samples + 1, num_circle_samples + 2);

		// middle triangle 2
		glm::vec3 t3(num_circle_samples + 1, num_circle_samples + 2, num_circle_samples * 2 + 1);

		// bottom cap triangle between start of circle and end
		glm::vec3 t4(1, num_circle_samples + 2, num_circle_samples * 2 + 1);

		for (int k = 0; k < 3; k++) {
			face[(num_circle_samples - 1) * face_att * 4 + k] = (GLuint)t1[k];
			face[(num_circle_samples - 1) * face_att * 4 + k + face_att] = (GLuint)t2[k];
			face[(num_circle_samples - 1) * face_att * 4 + k + face_att * 2] = (GLuint)t3[k];
			face[(num_circle_samples - 1) * face_att * 4 + k + face_att * 3] = (GLuint)t4[k];
		}

		// Create OpenGL buffers and copy data
		// Create buffer for vertices
		glGenBuffers(1, &cylVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, cylVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

		// Create buffer for faces
		glGenBuffers(1, &cylFaceBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylFaceBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

		// Free data buffers
		delete[] vertex;
		delete[] face;

		// Return number of elements in array buffer
		return face_num * face_att;
	}

	void Helicopter::switchBuffer(GLuint vertexBuffer, GLuint faceBuffer, GLuint shader, int atts) {
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		GLint vertex_att = glGetAttribLocation(shader, "vertex");
		glVertexAttribPointer(vertex_att, 3, GL_FLOAT, GL_FALSE, atts * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(vertex_att);

		GLint normal_att = glGetAttribLocation(shader, "normal");
		glVertexAttribPointer(normal_att, 3, GL_FLOAT, GL_FALSE, atts * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normal_att);

		GLint color_att = glGetAttribLocation(shader, "color");
		glVertexAttribPointer(color_att, 3, GL_FLOAT, GL_FALSE, atts * sizeof(GLfloat), (void *)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(color_att);

		if (atts == 11) {
			GLint tex_att = glGetAttribLocation(shader, "uv");
			glVertexAttribPointer(tex_att, 2, GL_FLOAT, GL_FALSE, atts * sizeof(GLfloat), (void *)(9 * sizeof(GLfloat)));
			glEnableVertexAttribArray(tex_att);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceBuffer);
		}
	}
	
	SceneNode* Helicopter::initHeli(ResourceManager * resman, SceneGraph *scene) {

		body = CreateInstance("BunBody", "BunBodMaterial", "Texture", "BunBodTexture", resman, scene);
		ears = CreateInstance("BunEars", "BunEarMaterial", "Texture", "BunEarTexture", resman, scene);
		ears->Translate(glm::vec3(0, 15, 0));
		ears->Rotate(glm::angleAxis(glm::radians(90.0f), glm::vec3(0.f, 1.f, 0.f)));

		SceneNode* n = new SceneNode("bunny", NULL, NULL);
		n->AddChild(body);
		body->AddChild(ears);
		return n;
	}

	SceneNode *Helicopter::CreateInstance(std::string entity_name, std::string object_name, std::string material_name, std::string texture_name,
		ResourceManager *resman_, SceneGraph *scene_) {

		Resource *geom = resman_->GetResource(object_name);
		if (!geom) {
			//throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = resman_->GetResource(material_name);
		if (!mat) {
			//throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}

		Resource *tex = resman_->GetResource(texture_name);
		if (!tex) {
			//issue
		}

		SceneNode *scn = new SceneNode(entity_name, geom, mat, tex);
		return scn;
	}

	void Helicopter::DrawHelicopter(GLuint program, Camera *camera) {
		if (first) { //only generate the meshes on the first run
			cyl_size = CreateCylinder();
			size = CreateCube();
			first = false;
		}

		glUseProgram(program); //we steal this program from another resource
		switchBuffer(cubeVertexBuffer, cubeFaceBuffer, program, 9);
		camera->SetupShader(program);
		SetupShader(program);

		GLint world_mat = glGetUniformLocation(program, "world_mat");

		// Set view matrix in shader
		GLint view_mat = glGetUniformLocation(program, "view_mat");
		glUniformMatrix4fv(view_mat, 1, GL_FALSE, glm::value_ptr(view_matrix_));

		// Set projection matrix in shader
		GLint projection_mat = glGetUniformLocation(program, "projection_mat");
		glUniformMatrix4fv(projection_mat, 1, GL_FALSE, glm::value_ptr(projection_matrix_));

		glm::mat4 base = glm::translate(glm::mat4(1.0), glm::vec3(0, -5.5, 740)) * glm::rotate(glm::mat4(1.0), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));

		glm::mat4 local;
		// ISROT or TORSI since reverse
		// body
		local = base * glm::scale(glm::mat4(1.0), glm::vec3(2.0, 2.0, 6.0));
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local));
		switchBuffer(cubeVertexBuffer, cubeFaceBuffer, program, 9);
		glDrawArrays(GL_TRIANGLES, 0, size);

		// cockpit
		local = base * glm::translate(glm::mat4(1.0), glm::vec3(0, -0.5, 3.5)) * glm::scale(glm::mat4(1.0), glm::vec3(2.0, 1.0, 1.0));
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local));
		glDrawArrays(GL_TRIANGLES, 0, size);

		glm::mat4 parent;

		// top rotor base
		parent = base * glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.25, 0.0));
		local = parent * glm::scale(glm::mat4(1.0), glm::vec3(2.0, 0.5, 2.0));
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local));
		switchBuffer(cylVertexBuffer, cylFaceBuffer, program, 11);
		glDrawElements(GL_TRIANGLES, cyl_size, GL_UNSIGNED_INT, 0);

		// top rotor blade
		local = parent * glm::mat4_cast(topOrientation) * glm::rotate(glm::mat4(1.0), glm::pi<float>() / 2, glm::vec3(0.0, 0.0, 1.0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.2, 12.0, 0.2));
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local));
		glDrawElements(GL_TRIANGLES, cyl_size, GL_UNSIGNED_INT, 0);

		// back rotor base
		parent = base * glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -5));
		local = parent * glm::rotate(glm::mat4(1.0), glm::pi<float>() / 2, glm::vec3(1.0, 0.0, 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.75, 4.0, 0.75));
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local));
		glDrawElements(GL_TRIANGLES, cyl_size, GL_UNSIGNED_INT, 0);

		// back rotor
		local = parent * glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0, -1.5)) * glm::mat4_cast(backOrientation) * glm::scale(glm::mat4(1.0), glm::vec3(0.1, 3.0, 0.1));
		glUniformMatrix4fv(world_mat, 1, GL_FALSE, glm::value_ptr(local));
		glDrawElements(GL_TRIANGLES, cyl_size, GL_UNSIGNED_INT, 0);
	}

} // namespace game
