#ifndef HELICOPTER_H_
#define HELICOPTER_H_
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "resource_manager.h"
#include "camera.h"
#include "scene_graph.h"

namespace game {
	// Abstraction of a camera
	class Helicopter {
	public:
		Helicopter(void);
		~Helicopter();

		// Get global camera attributes
		glm::vec3 GetPosition(void) const;
		glm::quat GetOrientation(void) const;

		// Set global camera attributes
		void SetPosition(glm::vec3 position);
		void SetOrientation(glm::quat orientation);
		void Translate(glm::vec3 trans);

		// here we go boys
		float vel_x = 0, vel_y = 0, vel_z = 0;
		void Update(double timer);

		// Get relative attributes of camera
		glm::vec3 GetForward(void) const;
		glm::vec3 GetSide(void) const;
		glm::vec3 GetUp(void) const;
		
		SceneNode *body;
		SceneNode *ears;

		void SetView(glm::vec3 position, glm::vec3 look_at, glm::vec3 up);
		void SetProjection(GLfloat fov, GLfloat near, GLfloat far, GLfloat w, GLfloat h);
		void SetupShader(GLuint program);

		//this is where the shit hits the fan
		bool first = true; //ensures we only generate the mesh for the heli once
		GLuint cubeVertexBuffer;
		GLuint cubeFaceBuffer;
		GLuint cylVertexBuffer;
		GLuint cylFaceBuffer;
		int cyl_size = -1;
		int size = -1;
		glm::quat topOrientation;
		glm::quat backOrientation;

		int CreateCube(void);
		int CreateCylinder(float cylinder_height = 1, float circle_radius = 0.5, int num_circle_samples = 30);
		void switchBuffer(GLuint vertexBuffer, GLuint faceBuffer, GLuint shader, int atts);

		SceneNode* initHeli(ResourceManager *resman, SceneGraph *scene);
		SceneNode* CreateInstance(std::string entity_name, std::string object_name, std::string material_name, std::string texture_name, ResourceManager *resman_, SceneGraph *scene_);
		
		void DrawHelicopter(GLuint program, Camera *camera);

	private:
		glm::vec3 position_; // Position of camera
		glm::quat orientation_; // Orientation of camera
		glm::vec3 forward_; // Initial forward vector
		glm::vec3 side_; // Initial side vector
		glm::mat4 view_matrix_; // View matrix
		glm::mat4 projection_matrix_; // Projection matrix
		void SetupViewMatrix(void); // Create view matrix from current camera parameters
	}; // class Camera
} // namespace game
#endif // CAMERA_H_
