#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SOIL/SOIL.h>
#include <string>
#include <array>
#include <vector>
#include <iterator>
#include "resource_manager.h"
#include "bin/path_config.h"

namespace game {
	ResourceManager::ResourceManager(void) {}

	ResourceManager::~ResourceManager() {}

	void ResourceManager::AddResource(ResourceType type, const std::string name, GLuint resource, GLsizei size) {
		Resource *res;

		res = new Resource(type, name, resource, size);

		resource_.push_back(res);
	}

	void ResourceManager::AddResource(ResourceType type, const std::string name, GLuint array_buffer, GLuint element_array_buffer, GLsizei size, Hitbox _hb) {
		Resource *res;

		res = new Resource(type, name, array_buffer, element_array_buffer, size, _hb);

		resource_.push_back(res);
	}

	void ResourceManager::LoadResource(ResourceType type, const std::string name, const char *filename) {
		// Call appropriate method depending on type of resource
		if (type == Material) {
			LoadMaterial(name, filename);
		}
		else if (type == Texture) {
			LoadTexture(name, filename);
		}
		else {
			throw(std::invalid_argument(std::string("Invalid type of resource")));
		}
	}

	void ResourceManager::LoadTexture(const std::string name, const char *filename) {
		// Load texture from file
		GLuint texture = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
		if (!texture) {
			throw(std::ios_base::failure(std::string("Error loading texture ") + std::string(filename) + std::string(": ") + std::string(SOIL_last_result())));
		}

		// Create resource
		AddResource(Texture, name, texture, 0);
	}

	Resource *ResourceManager::GetResource(const std::string name) const {
		// Find resource with the specified name
		for (int i = 0; i < resource_.size(); i++) {
			if (resource_[i]->GetName() == name) {
				return resource_[i];
			}
		}
		return NULL;
	}

	void ResourceManager::LoadMaterial(const std::string name, const char *prefix) {

		// Load vertex program source code
		std::string filename = std::string(prefix) + std::string(VERTEX_PROGRAM_EXTENSION);
		std::string vp = LoadTextFile(filename.c_str());

		// Load fragment program source code
		filename = std::string(prefix) + std::string(FRAGMENT_PROGRAM_EXTENSION);
		std::string fp = LoadTextFile(filename.c_str());

		// Create a shader from the vertex program source code
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		const char *source_vp = vp.c_str();
		glShaderSource(vs, 1, &source_vp, NULL);
		glCompileShader(vs);

		// Check if shader compiled successfully
		GLint status;
		glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			char buffer[512];
			glGetShaderInfoLog(vs, 512, NULL, buffer);
			throw(std::ios_base::failure(std::string("Error compiling vertex shader: ") + std::string(buffer)));
		}

		// Create a shader from the fragment program source code
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		const char *source_fp = fp.c_str();
		glShaderSource(fs, 1, &source_fp, NULL);
		glCompileShader(fs);

		// Check if shader compiled successfully
		glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			char buffer[512];
			glGetShaderInfoLog(fs, 512, NULL, buffer);
			throw(std::ios_base::failure(std::string("Error compiling fragment shader: ") + std::string(buffer)));
		}

		// Try to also load a geometry shader
		filename = std::string(prefix) + std::string(GEOMETRY_PROGRAM_EXTENSION);
		bool geometry_program = false;
		std::string gp = "";
		GLuint gs;
		try {
			gp = LoadTextFile(filename.c_str());
			geometry_program = true;
		}
		catch (std::exception &e) {
		}

		if (geometry_program) {
			// Create a shader from the geometry program source code
			gs = glCreateShader(GL_GEOMETRY_SHADER);
			const char *source_gp = gp.c_str();
			glShaderSource(gs, 1, &source_gp, NULL);
			glCompileShader(gs);

			// Check if shader compiled successfully
			GLint status;
			glGetShaderiv(gs, GL_COMPILE_STATUS, &status);
			if (status != GL_TRUE) {
				char buffer[512];
				glGetShaderInfoLog(gs, 512, NULL, buffer);
				throw(std::ios_base::failure(std::string("Error compiling geometry shader: ") + std::string(buffer)));
			}
		}

		// Create a shader program linking both vertex and fragment shaders
		GLuint sp = glCreateProgram();
		glAttachShader(sp, vs);
		glAttachShader(sp, fs);
		if (geometry_program) {
			glAttachShader(sp, gs);
		}
		glLinkProgram(sp);

		// Check if shaders were linked successfully
		glGetProgramiv(sp, GL_LINK_STATUS, &status);
		if (status != GL_TRUE) {
			char buffer[512];
			glGetShaderInfoLog(sp, 512, NULL, buffer);
			throw(std::ios_base::failure(std::string("Error linking shaders: ") + std::string(buffer)));
		}

		// Delete memory used by shaders, since they were already compiled
		glDeleteShader(vs);
		glDeleteShader(fs);
		if (geometry_program) {
			glDeleteShader(gs);
		}

		// Add a resource for the shader program
		AddResource(Material, name, sp, 0);
	}

	std::string ResourceManager::LoadTextFile(const char *filename) {
		// Open file
		std::ifstream f;
		f.open(filename);
		if (f.fail()) {
			throw(std::ios_base::failure(std::string("Error opening file ") + std::string(filename)));
		}

		// Read file
		std::string content;
		std::string line;
		while (std::getline(f, line)) {
			content += line + "\n";
		}

		// Close file
		f.close();

		return content;
	}

	Hitbox ResourceManager::genHitbox(std::vector<glm::vec3> points)
	{
		// need to take the maximum and minimum values of the model, and return that box

		// right/left = +/- x
		// up/down = +/- y 
		// close/far = +/- z
		float right = NAN, left = NAN, up = NAN, down = NAN, close = NAN, Far = NAN;
		for (glm::vec3 p : points) {
			right = fmax(right, p.x);
			left = fmin(left, p.x);

			up = fmax(up, p.y);
			down = fmin(down, p.y);

			close = fmax(close, p.z);
			Far = fmin(Far, p.z);
		}

		std::vector<glm::vec3> hb_points = std::vector<glm::vec3>();
		hb_points.push_back(glm::vec3(left, down, Far));
		hb_points.push_back(glm::vec3(left, down, close));
		hb_points.push_back(glm::vec3(left, up, Far));
		hb_points.push_back(glm::vec3(left, up, close));
		hb_points.push_back(glm::vec3(right, down, Far));
		hb_points.push_back(glm::vec3(right, down, close));
		hb_points.push_back(glm::vec3(right, up, Far));
		hb_points.push_back(glm::vec3(right, up, close));
		Hitbox hb = Hitbox(hb_points);

		return hb;
	}

	void ResourceManager::CreateTorus(std::string object_name, float loop_radius, float circle_radius, int num_loop_samples, int num_circle_samples) {
		// Number of vertices and faces to be created
		// Check the construction algorithm below to understand the numbers
		// specified below
		const GLuint vertex_num = num_loop_samples*num_circle_samples;
		const GLuint face_num = num_loop_samples*num_circle_samples * 2;

		// Number of attributes for vertices and faces
		const int vertex_att = 11;
		const int face_att = 3;

		// Data buffers for the torus
		GLfloat *vertex = NULL;
		GLuint *face = NULL;

		// Allocate memory for buffers
		try {
			vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
			face = new GLuint[face_num * face_att]; // 3 indices per face
		}
		catch (std::exception &e) {
			throw e;
		}

		// Create vertices 
		float theta, phi; // Angles for circles
		glm::vec3 loop_center;
		glm::vec3 vertex_position;
		glm::vec3 vertex_normal;
		glm::vec3 vertex_color;
		glm::vec2 vertex_coord;

		// for generating hitbox
		std::vector<glm::vec3> all_positions = std::vector<glm::vec3>();

		for (int i = 0; i < num_loop_samples; i++) { // large loop

			theta = 2.0*glm::pi<GLfloat>()*i / num_loop_samples; // loop sample (angle theta)
			loop_center = glm::vec3(loop_radius*cos(theta), loop_radius*sin(theta), 0); // centre of a small circle

			for (int j = 0; j < num_circle_samples; j++) { // small circle

				phi = 2.0*glm::pi<GLfloat>()*j / num_circle_samples; // circle sample (angle phi)

				// Define position, normal and color of vertex
				vertex_normal = glm::vec3(cos(theta)*cos(phi), sin(theta)*cos(phi), sin(phi));
				vertex_position = loop_center + vertex_normal*circle_radius;
				vertex_color = glm::vec3(1.0 - ((float)i / (float)num_loop_samples),
					(float)i / (float)num_loop_samples,
					(float)j / (float)num_circle_samples);
				vertex_coord = glm::vec2(theta / 2.0*glm::pi<GLfloat>(),
					phi / 2.0*glm::pi<GLfloat>());

				all_positions.push_back(vertex_position);

				// Add vectors to the data buffer
				for (int k = 0; k < 3; k++) {
					vertex[(i*num_circle_samples + j)*vertex_att + k] = vertex_position[k];
					vertex[(i*num_circle_samples + j)*vertex_att + k + 3] = vertex_normal[k];
					vertex[(i*num_circle_samples + j)*vertex_att + k + 6] = vertex_color[k];
				}
				vertex[(i*num_circle_samples + j)*vertex_att + 9] = vertex_coord[0];
				vertex[(i*num_circle_samples + j)*vertex_att + 10] = vertex_coord[1];
			}
		}

		// Create triangles
		for (int i = 0; i < num_loop_samples; i++) {
			for (int j = 0; j < num_circle_samples; j++) {
				// Two triangles per quad
				glm::vec3 t1(((i + 1) % num_loop_samples)*num_circle_samples + j,
					i*num_circle_samples + ((j + 1) % num_circle_samples),
					i*num_circle_samples + j);
				glm::vec3 t2(((i + 1) % num_loop_samples)*num_circle_samples + j,
					((i + 1) % num_loop_samples)*num_circle_samples + ((j + 1) % num_circle_samples),
					i*num_circle_samples + ((j + 1) % num_circle_samples));
				// Add two triangles to the data buffer
				for (int k = 0; k < 3; k++) {
					face[(i*num_circle_samples + j)*face_att * 2 + k] = (GLuint)t1[k];
					face[(i*num_circle_samples + j)*face_att * 2 + k + face_att] = (GLuint)t2[k];
				}
			}
		}

		// Create OpenGL buffers and copy data
		GLuint vbo, ebo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

		// Free data buffers
		delete[] vertex;
		delete[] face;

		// Create resource
		AddResource(Mesh, object_name, vbo, ebo, face_num * face_att, genHitbox(all_positions));
	}

	void ResourceManager::CreateSphere(std::string object_name, float radius, int num_samples_theta, int num_samples_phi) {
		// Create a sphere using a well-known parameterization
		// Number of vertices and faces to be created
		const GLuint vertex_num = num_samples_theta*num_samples_phi;
		const GLuint face_num = num_samples_theta*(num_samples_phi - 1) * 2;

		// Number of attributes for vertices and faces
		const int vertex_att = 11;
		const int face_att = 3;

		// Data buffers 
		GLfloat *vertex = NULL;
		GLuint *face = NULL;

		// Allocate memory for buffers
		try {
			vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
			face = new GLuint[face_num * face_att]; // 3 indices per face
		}
		catch (std::exception &e) {
			throw e;
		}

		// Create vertices 
		float theta, phi; // Angles for parametric equation
		glm::vec3 vertex_position;
		glm::vec3 vertex_normal;
		glm::vec3 vertex_color;
		glm::vec2 vertex_coord;

		std::vector<glm::vec3> positions = std::vector<glm::vec3>(); // for generating hitbox

		for (int i = 0; i < num_samples_theta; i++) {
			theta = 2.0*glm::pi<GLfloat>()*i / (num_samples_theta - 1); // angle theta
			for (int j = 0; j < num_samples_phi; j++) {
				phi = glm::pi<GLfloat>()*j / (num_samples_phi - 1); // angle phi

				// Define position, normal and color of vertex
				vertex_normal = glm::vec3(cos(theta)*sin(phi), sin(theta)*sin(phi), -cos(phi));
				// We need z = -cos(phi) to make sure that the z coordinate runs from -1 to 1 as phi runs from 0 to pi
				// Otherwise, the normal will be inverted
				vertex_position = glm::vec3(vertex_normal.x*radius,
					vertex_normal.y*radius,
					vertex_normal.z*radius),
					vertex_color = glm::vec3(0.8, 0.7, 0.1);
				//((float)i) / ((float)num_samples_theta), 1.0 - ((float)j) / ((float)num_samples_phi), ((float)j) / ((float)num_samples_phi)
				vertex_coord = glm::vec2(((float)i) / ((float)num_samples_theta), 1.0 - ((float)j) / ((float)num_samples_phi));

				positions.push_back(vertex_position);

				// Add vectors to the data buffer
				for (int k = 0; k < 3; k++) {
					vertex[(i*num_samples_phi + j)*vertex_att + k] = vertex_position[k];
					vertex[(i*num_samples_phi + j)*vertex_att + k + 3] = vertex_normal[k];
					vertex[(i*num_samples_phi + j)*vertex_att + k + 6] = vertex_color[k];
				}
				vertex[(i*num_samples_phi + j)*vertex_att + 9] = vertex_coord[0];
				vertex[(i*num_samples_phi + j)*vertex_att + 10] = vertex_coord[1];
			}
		}

		// Create faces
		for (int i = 0; i < num_samples_theta; i++) {
			for (int j = 0; j < (num_samples_phi - 1); j++) {
				// Two triangles per quad
				glm::vec3 t1(((i + 1) % num_samples_theta)*num_samples_phi + j,
					i*num_samples_phi + (j + 1),
					i*num_samples_phi + j);
				glm::vec3 t2(((i + 1) % num_samples_theta)*num_samples_phi + j,
					((i + 1) % num_samples_theta)*num_samples_phi + (j + 1),
					i*num_samples_phi + (j + 1));
				// Add two triangles to the data buffer
				for (int k = 0; k < 3; k++) {
					face[(i*(num_samples_phi - 1) + j)*face_att * 2 + k] = (GLuint)t1[k];
					face[(i*(num_samples_phi - 1) + j)*face_att * 2 + k + face_att] = (GLuint)t2[k];
				}
			}
		}

		// Create OpenGL buffers and copy data
		GLuint vbo, ebo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

		// Free data buffers
		delete[] vertex;
		delete[] face;

		// Create resource
		AddResource(Mesh, object_name, vbo, ebo, face_num * face_att, genHitbox(positions));
	}

	void ResourceManager::CreateCube(std::string object_name) {

		// This construction uses shared vertices, following the same data format as the other functions
		// However, vertices are repeated since their normals at each face are different
		// Each face of the cube is defined by four vertices (with the same normal) and two triangles

		// Vertices used to build the cube
		// 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), texture coordinates (2)
		GLfloat vertex[] = {
			// First cube face 
			-0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.1, 0.1,    0.0, 0.0,
			0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.1, 0.1,    1.0, 0.0,
			0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.1, 0.1,    1.0, 1.0,
			-0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    0.4, 0.1, 0.1,    0.0, 1.0,
			// Second cube face
			0.5, -0.5, -0.5,    1.0,  0.0,  0.0,    0.1, 0.45, 0.45,    0.0, 0.0,
			0.5,  0.5, -0.5,    1.0,  0.0,  0.0,    0.1, 0.45, 0.45,    1.0, 0.0,
			0.5,  0.5,  0.5,    1.0,  0.0,  0.0,    0.1, 0.45, 0.45,    1.0, 1.0,
			0.5, -0.5,  0.5,    1.0,  0.0,  0.0,    0.1, 0.45, 0.45,    0.0, 1.0,
			// Third cube face
			0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.4, 0.1, 0.4,    0.0, 0.0,
			-0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.4, 0.1, 0.4,    1.0, 0.0,
			-0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.4, 0.1, 0.4,    1.0, 1.0,
			0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.4, 0.1, 0.4,    0.0, 1.0,
			// Fourth cube face
			-0.5,  0.5, -0.5,   -1.0,  0.0,  0.0,    0.45, 0.45, 0.1,    0.0, 0.0,
			-0.5, -0.5, -0.5,   -1.0,  0.0,  0.0,    0.45, 0.45, 0.1,    1.0, 0.0,
			-0.5, -0.5,  0.5,   -1.0,  0.0,  0.0,    0.45, 0.45, 0.1,    1.0, 1.0,
			-0.5,  0.5,  0.5,   -1.0,  0.0,  0.0,    0.45, 0.45, 0.1,    0.0, 1.0,
			// Fifth cube face
			-0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    0.1, 0.1, 0.55,    0.0, 0.0,
			-0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.1, 0.1, 0.55,    0.0, 1.0,
			0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.1, 0.1, 0.55,    1.0, 1.0,
			0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    0.1, 0.1, 0.55,    1.0, 0.0,
			// Sixth cube face
			0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    0.1, 0.55, 0.1,    0.0, 0.0,
			-0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    0.1, 0.55, 0.1,    1.0, 0.0,
			-0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    0.1, 0.55, 0.1,    1.0, 1.0,
			0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    0.1, 0.55, 0.1,    0.0, 1.0,
		};

		std::vector<glm::vec3> positions = std::vector<glm::vec3>();
		positions.push_back(glm::vec3(-0.5, -0.5, -0.5));
		positions.push_back(glm::vec3(-0.5, -0.5, 0.5));
		positions.push_back(glm::vec3(-0.5, 0.5, -0.5));
		positions.push_back(glm::vec3(-0.5, 0.5, 0.5));
		positions.push_back(glm::vec3(0.5, -0.5, -0.5));
		positions.push_back(glm::vec3(0.5, -0.5, 0.5));
		positions.push_back(glm::vec3(0.5, 0.5, -0.5));
		positions.push_back(glm::vec3(0.5, 0.5, 0.5));

		// Triangles
		GLuint face[] = {
			// First cube face, with two triangles
			0, 1, 2,
			0, 2, 3,
			// Second face
			4, 5, 6,
			4, 6, 7,
			// Third face
			8, 9, 10,
			8, 10, 11,
			// Fourth face
			12, 13, 14,
			12, 14, 15,
			// Fifth face
			16, 17, 18,
			16, 18, 19,
			// Sixth face
			20, 21, 22,
			20, 22, 23,
		};

		// Create OpenGL buffers and copy data
		GLuint vbo, ebo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face), face, GL_STATIC_DRAW);

		// Create resource
		AddResource(Mesh, object_name, vbo, ebo, sizeof(face) / sizeof(GLfloat), genHitbox(positions));
	}

	void ResourceManager::CreateGround(std::string object_name) {
		struct Vertex {
			glm::vec3 vertex_position;
			glm::vec3 vertex_normal;
			glm::vec3 vertex_color;
			glm::vec2 vertex_coord;
		};

		std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/heightmap.bmp");
		const char *const file = filename.c_str();
		cimg_library::CImg<> img;
		img.assign(file);

		// Number of vertices and faces to be created
		const GLuint vertex_num = img.width() * img.height();
		const GLuint face_num = vertex_num * 2;

		std::vector<std::vector<Vertex>> vertices = std::vector<std::vector<Vertex>>();

		// Number of attributes for vertices and faces
		const int vertex_att = 11; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
		const int face_att = 3; // 3 indices per face

		// Data buffers 
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

		Vertex v;
		for (float i = 0; i < img.height(); i++) {
			vertices.push_back(std::vector<Vertex>());
			for (float j = 0; j < img.width(); j++) {

				v.vertex_position = glm::vec3(i * 2, (img.atXY(i, j)) / 5, j * 2);
				v.vertex_normal = glm::vec3(0, 0, 0);
				v.vertex_color = glm::vec3(0, 1.0 - (img.atXY(i, j) / 255), 0);

				// fix texture coords :(
				v.vertex_coord = glm::vec2(i / img.height(), j / img.width());

				vertices[i].push_back(v);
			}
		}

		// Create faces
		glm::vec3 t1, t2;
		glm::vec3 face_normal;
		int index1, index2, index3;
		Vertex *v1, *v2, *v3;
		glm::vec3 e1, e2;
		for (int i = 0; i < img.height() - 1; i++) {
			for (int j = 0; j < (img.width() - 1); j++) {
				// get each vertex of triangle
				v1 = &vertices[i][j];
				v2 = &vertices[i][j + 1];
				v3 = &vertices[i + 1][j];

				// calculate the normal
				e1 = v2->vertex_position - v1->vertex_position;
				e2 = v3->vertex_position - v1->vertex_position;
				face_normal = glm::cross(e1, e2);
				face_normal = glm::normalize(face_normal);

				// apply the normals to the vertices
				v1->vertex_normal += face_normal;
				v2->vertex_normal += face_normal;
				v3->vertex_normal += face_normal;

				// and now do it for the second triangle
				// get each vertex of triangle
				v1 = &vertices[i + 1][j + 1];
				v2 = &vertices[i][j + 1];
				v3 = &vertices[i + 1][j];

				// calculate the normal
				e1 = v2->vertex_position - v1->vertex_position;
				e2 = v3->vertex_position - v1->vertex_position;
				face_normal = glm::cross(e2, e1);
				face_normal = glm::normalize(face_normal);

				v1->vertex_normal += face_normal;
				v2->vertex_normal += face_normal;
				v3->vertex_normal += face_normal;

				// Two triangles per quad
				t1 = glm::vec3(i*img.width() + j, i*img.width() + j + 1, (i + 1)*img.width() + j);
				t2 = glm::vec3(i*img.width() + j + 1, (i + 1)*img.width() + j + 1, (i + 1)*img.width() + j);

				// Add two triangles to the data buffer
				for (int k = 0; k < 3; k++) {
					face[(i*(img.width() - 1) + j)*face_att * 2 + k] = (GLuint)t1[k];
					face[(i*(img.width() - 1) + j)*face_att * 2 + k + face_att] = (GLuint)t2[k];
				}
			}
		}

		// now we're going to write all the vertices to buffers
		//Vertex v;
		for (int i = 0; i < img.height(); i++) {
			for (int j = 0; j < img.width(); j++) {
				v = vertices[i][j];
				v.vertex_normal = glm::normalize(v.vertex_normal);

				for (int k = 0; k < 3; k++) {
					vertex[(i*img.width() + j)*vertex_att + k] = v.vertex_position[k];
					vertex[(i*img.width() + j)*vertex_att + k + 3] = v.vertex_normal[k];
					vertex[(i*img.width() + j)*vertex_att + k + 6] = v.vertex_normal[k];
				}
				vertex[(i*img.width() + j)*vertex_att + 9] = v.vertex_coord[0];
				vertex[(i*img.width() + j)*vertex_att + 10] = v.vertex_coord[1];
			}
		}

		GLuint vbo, ebo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

		// Free data buffers
		delete[] vertex;
		delete[] face;

		// Create resource
		AddResource(Mesh, object_name, vbo, ebo, face_num * face_att, Hitbox());
	}

	void ResourceManager::CreateCylinder(std::string object_name, float cylinder_height, float circle_radius, int num_circle_samples) {
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

		std::vector<glm::vec3> positions = std::vector<glm::vec3>(); // for generating hitbox

		// vertices at
		// - one in center of each endcap
		// - num_circle_samples worth on edges of each endcap

		// top centerpoint
		vertex_normal = glm::vec3(0, 0, 1);
		vertex_position = glm::vec3(0, cylinder_height / 2, 0);
		vertex_color = glm::vec3(1.0, 0.0, 0.0);
		vertex_coord = glm::vec2(theta / 2.0*glm::pi<GLfloat>(),
			theta / 2.0*glm::pi<GLfloat>());

		positions.push_back(vertex_position);

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

		positions.push_back(vertex_position);

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

			positions.push_back(vertex_position);

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

			positions.push_back(vertex_position);

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
		GLuint vbo, ebo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

		// Create buffer for faces
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

		// Free data buffers
		delete[] vertex;
		delete[] face;

		// Return number of elements in array buffer
		AddResource(Mesh, object_name, vbo, ebo, face_num * face_att, genHitbox(positions));
	}

	void ResourceManager::CreateSphereParticles(std::string object_name, int num_particles) {

		// Create a set of points which will be the particles
		// This is similar to drawing a sphere: we will sample points on a sphere, but will allow them to also deviate a bit from the sphere along the normal (change of radius)

		// Data buffer
		GLfloat *particle = NULL;

		// Number of attributes per particle: position (3), normal (3), and color (3), texture coordinates (2)
		const int particle_att = 11;

		// Allocate memory for buffer
		try {
			particle = new GLfloat[num_particles * particle_att];
		}
		catch (std::exception &e) {
			throw e;
		}

		float trad = 0.2; // Defines the starting point of the particles along the normal
		float maxspray = 0.5; // This is how much we allow the points to deviate from the sphere
		float u, v, w, theta, phi, spray; // Work variables

		for (int i = 0; i < num_particles; i++) {

			// Get three random numbers
			u = ((double)rand() / (RAND_MAX));
			v = ((double)rand() / (RAND_MAX));
			w = ((double)rand() / (RAND_MAX));

			// Use u to define the angle theta along one direction of the sphere
			theta = u * 2.0*glm::pi<float>();
			// Use v to define the angle phi along the other direction of the sphere
			phi = acos(2.0*v - 1.0);
			// Use w to define how much we can deviate from the surface of the sphere (change of radius)
			spray = maxspray*pow((float)w, (float)(1.0 / 3.0)); // Cubic root of w

			// Define the normal and point based on theta, phi and the spray
			glm::vec3 normal(spray*cos(theta)*sin(phi), spray*sin(theta)*sin(phi), spray*cos(phi));
			glm::vec3 position(normal.x*trad, normal.y*trad, normal.z*trad);
			glm::vec3 color(i / (float)num_particles, 0.0, 1.0 - (i / (float)num_particles)); // We can use the color for debug, if needed

			for (int k = 0; k < 3; k++) {
				particle[i*particle_att + k] = position[k];
				particle[i*particle_att + k + 3] = normal[k];
				particle[i*particle_att + k + 6] = color[k];
			}
		}

		// Create OpenGL buffers and copy data
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, num_particles * particle_att * sizeof(GLfloat), particle, GL_STATIC_DRAW);

		// Free data buffers
		delete[] particle;

		// Create resource
		AddResource(PointSet, object_name, vbo, 0, num_particles);
	}
} // namespace game;
