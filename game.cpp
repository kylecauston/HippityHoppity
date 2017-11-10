#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "bin/path_config.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Main window settings
const std::string window_title_g = "Demo";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const bool window_full_screen_g = false;

// Viewport and camera settings
float camera_near_clip_distance_g = 0.01;
float camera_far_clip_distance_g = 1000.0;
float camera_fov_g = 20.0; // Field-of-view of camera
const glm::vec3 viewport_background_color_g(0.0, 0.3, 0.6);
glm::vec3 camera_position_g(0.0, 0.0, 800.0);
glm::vec3 camera_look_at_g(0.0, 0.0, 0.0);
glm::vec3 camera_up_g(0.0, 1.0, 0.0);

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


Game::Game(void){

    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void){

	srand(time(NULL));

    // Run all initialization steps
    InitWindow();
    InitView();
    InitEventHandlers();

    // Set variables
    animating_ = true;
}

       
void Game::InitWindow(void){

    // Initialize the window management library (GLFW)
    if (!glfwInit()){
        throw(GameException(std::string("Could not initialize the GLFW library")));
    }

    // Create a window and its OpenGL context
    if (window_full_screen_g){
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
    } else {
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
    }
    if (!window_){
        glfwTerminate();
        throw(GameException(std::string("Could not create window")));
    }

    // Make the window's context the current one
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        throw(GameException(std::string("Could not initialize the GLEW library: ")+std::string((const char *) glewGetErrorString(err))));
    }
}


void Game::InitView(void){

    // Set up z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set viewport
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    glViewport(0, 0, width, height);

    // Set up camera
    // Set current view
    camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
	heli_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
    // Set projection
    camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
	heli_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);

}


void Game::InitEventHandlers(void){

    // Set event callbacks
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Set pointer to game object, so that callbacks can access it
    glfwSetWindowUserPointer(window_, (void *) this);
}


void Game::SetupResources(void){

    // Create a simple sphere to represent the asteroids
    resman_.CreateSphere("SimpleSphereMesh", 1.0, 10, 10);

	resman_.CreateCube("CubePointSet"); //set up cube for the laser

    // Load material to be applied to asteroids
    std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

	//new asteroid texture - CLOUDS
	//textured material
	filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_texture");
	resman_.LoadResource(Material, "ShinyTextureMaterial", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/cloudtex.jpg");
	resman_.LoadResource(Texture, "Cloud", filename.c_str());
}


void Game::SetupScene(void){

    // Set background color for the scene
    scene_.SetBackgroundColor(viewport_background_color_g);

    // Create asteroid field
    CreateAsteroidField(200);

	prgm = resman_.GetResource("ObjectMaterial")->GetResource();
}


void Game::MainLoop(void){

    // Loop while the user did not close the window
    while (!glfwWindowShouldClose(window_)){
        // Animate the scene
        if (animating_){
            static double last_time = 0;
            double current_time = glfwGetTime();
            if ((current_time - last_time) > 0.05){
                scene_.Update();
				//update our camera to keep momentum going with thrusters
				camera_.Update();
				heli_.Update(current_time - last_time);
                last_time = current_time;
            }
        }

        // Draw the scene
        scene_.Draw(&camera_);

		
		heli_.DrawHelicopter(prgm, &camera_);
        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);

        // Update other events like input handling
        glfwPollEvents();
    }
}


void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Get user data with a pointer to the game class
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;

    // Quit game if 'q' is pressed
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    // Stop animation if space bar is pressed
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        game->animating_ = (game->animating_ == true) ? false : true;
    }

    // View control
	float deviation = 0.5 + ((rand() % 101) / 100); //ranges from 50% to 150% rotation speed
    float rot_factor(deviation * glm::pi<float>() / 180); //deviates a bit to add realism?
    float trans_factor = 0.5;
    if (key == GLFW_KEY_UP){
        game->camera_.Pitch(rot_factor);
    }
    if (key == GLFW_KEY_DOWN){
        game->camera_.Pitch(-rot_factor);
    }
    if (key == GLFW_KEY_LEFT){
        game->camera_.Yaw(rot_factor);
    }
    if (key == GLFW_KEY_RIGHT){
        game->camera_.Yaw(-rot_factor);
    }
    if (key == GLFW_KEY_S){
        game->camera_.Roll(-rot_factor);
    }
    if (key == GLFW_KEY_X){
        game->camera_.Roll(rot_factor);
    }
    if (key == GLFW_KEY_A){ //translation movements are based on an accelerating velocity now
		game->camera_.vel_z += trans_factor;
    }
    if (key == GLFW_KEY_Z){
		game->camera_.vel_z -= trans_factor;
    }
    if (key == GLFW_KEY_J){
		game->camera_.vel_x -= trans_factor;
    }
    if (key == GLFW_KEY_L){
		game->camera_.vel_x += trans_factor;
    }
    if (key == GLFW_KEY_I){
		game->camera_.vel_y += trans_factor;
    }
    if (key == GLFW_KEY_K){
		game->camera_.vel_y -= trans_factor;
    }
	if (key == GLFW_KEY_W) { //always good to have a brake handy
		game->camera_.vel_x = 0;
		game->camera_.vel_y = 0;
		game->camera_.vel_z = 0;
	}
	if (key == GLFW_KEY_E) { //fire!
		game->FireLaser();
	}
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height){

    // Set up viewport and camera projection based on new window size
    glViewport(0, 0, width, height);
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;
    game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
	game->heli_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
}


Game::~Game(){
    
    glfwTerminate();
}


Asteroid *Game::CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name, std::string tex_name){

    // Get resources
    Resource *geom = resman_.GetResource(object_name);
    if (!geom){
        throw(GameException(std::string("Could not find resource \"")+object_name+std::string("\"")));
    }

    Resource *mat = resman_.GetResource(material_name);
    if (!mat){
        throw(GameException(std::string("Could not find resource \"")+material_name+std::string("\"")));
    }

	Resource *tex = NULL;
	if (tex_name != "") {
		tex = resman_.GetResource(tex_name);
		if (!tex) {
			throw(GameException(std::string("Could not find resource \"") + tex_name + std::string("\"")));
		}
	}

    // Create asteroid instance
    Asteroid *ast = new Asteroid(entity_name, geom, mat, tex);
    scene_.AddNode(ast);
    return ast;
}

Laser *Game::LaserCube(std::string entity_name, std::string object_name, std::string material_name) {

	// Get resources
	Resource *geom = resman_.GetResource(object_name);
	if (!geom) {
		throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
	}

	Resource *mat = resman_.GetResource(material_name);
	if (!mat) {
		throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
	}

	// Create asteroid instance
	Laser *cube = new Laser(entity_name, geom, mat);
	scene_.AddNode(cube);
	return cube;
}


void Game::CreateAsteroidField(int num_asteroids){

    // Create a number of asteroid instances
    for (int i = 0; i < num_asteroids; i++){
        // Create instance name
        std::stringstream ss;
        ss << i;
        std::string index = ss.str();
        std::string name = "AsteroidInstance" + index;

        // Create asteroid instance
        //Asteroid *ast = CreateAsteroidInstance(name, "SimpleSphereMesh", "ObjectMaterial");
		Asteroid *ast = CreateAsteroidInstance(name, "SimpleSphereMesh", "ShinyTextureMaterial", "Cloud");

        // Set attributes of asteroid: random position, orientation, and
        // angular momentum
        ast->SetPosition(glm::vec3(-300.0 + 600.0*((float) rand() / RAND_MAX), -300.0 + 600.0*((float) rand() / RAND_MAX), 600.0*((float) rand() / RAND_MAX)));
        ast->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
        ast->SetAngM(glm::normalize(glm::angleAxis(0.05f*glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
    }
}

void Game::FireLaser() {
	scene_.RemoveLast("Laser1"); //only one laser can exist at once
	Laser *cube = LaserCube("Laser1", "CubePointSet", "ObjectMaterial");
	cube->SetPosition(camera_.GetPosition()); //fires outward from the player's position
	cube->SetOrientation(camera_.GetOrientation());
	cube->SetDir(camera_.GetForward());
	cube->SetScale(glm::vec3(0.2, 0.2, 0.55));

	
	std::string collide = RaySphere(camera_.GetForward(), camera_.GetPosition());
	if (collide != "none") {
		scene_.RemoveLast(collide); //blow up the asteroid we collided with
	}
}

std::string Game::RaySphere(glm::vec3 raydir, glm::vec3 raypos) {
	for (int i = 0; i < scene_.node_.size()-1; i++) { //check against all asteroids
		glm::vec3 d = scene_.node_[i]->GetPosition() - raypos;
		float projectD = glm::dot(d, raydir);
		float d2 = glm::dot(d, d) - projectD * projectD;
		if (d2 > 1.0) continue; //can't collide with this sphere, move on
		float d3 = sqrt(1.0 - d2);

		//possible solutions
		float s1 = projectD - d3;
		float s2 = projectD + d3;
		if (s1 >= 0 || s2 >= 0) { //we found one, return the name
			return scene_.node_[i]->GetName();
		}
	}
	return "none"; //didn't find anything
}

} // namespace game
