#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
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

	bool temp = false;
	glm::vec3 vel(0, 0, 0);

	// Viewport and camera settings
	float camera_near_clip_distance_g = 0.01;
	float camera_far_clip_distance_g = 1000.0;
	float camera_fov_g = 20.0; // Field-of-view of camera
	const glm::vec3 viewport_background_color_g(0.0, 0.3, 0.6); //dark blue sky background
	glm::vec3 camera_position_g(0.0, 0.0, 800.0);
	glm::vec3 camera_look_at_g(0.0, 0.0, 0.0);
	glm::vec3 camera_up_g(0.0, 1.0, 0.0);

	// Materials 
	const std::string material_directory_g = MATERIAL_DIRECTORY;

	Game::Game(void) {
		// Don't do work in the constructor, leave it for the Init() function
	}

	void Game::Init(void) {
		srand(time(NULL));

		// Run all initialization steps
		InitWindow();
		InitView();
		InitEventHandlers();

		// Set variables
		animating_ = true;
		game_state = TITLE;
	}

	void Game::InitWindow(void) {
		// Initialize the window management library (GLFW)
		if (!glfwInit()) {
			throw(GameException(std::string("Could not initialize the GLFW library")));
		}

		// Create a window and its OpenGL context
		if (window_full_screen_g) {
			window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
		}
		else {
			window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
		}
		if (!window_) {
			glfwTerminate();
			throw(GameException(std::string("Could not create window")));
		}

		// Make the window's context the current one
		glfwMakeContextCurrent(window_);

		// Initialize the GLEW library to access OpenGL extensions
		// Need to do it after initializing an OpenGL context
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			throw(GameException(std::string("Could not initialize the GLEW library: ") + std::string((const char *)glewGetErrorString(err))));
		}
	}

	void Game::InitView(void) {
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

	void Game::InitEventHandlers(void) {
		// Set event callbacks
		glfwSetKeyCallback(window_, KeyCallback);
		glfwSetFramebufferSizeCallback(window_, ResizeCallback);

		// Set pointer to game object, so that callbacks can access it
		glfwSetWindowUserPointer(window_, (void *) this);
	}

	void Game::SetupResources(void) {
		// Create our meshes
		resman_.CreateSphere("SimpleSphereMesh", 1.0, 10, 10);

		resman_.CreateCube("CubePointSet"); //set up cube for the laser

		resman_.CreateGround("Terrain"); // load terrain

		// Load non textured materials
		std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
		resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

		//load texture materials
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_texture");
		resman_.LoadResource(Material, "ShinyTextureMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/cloudtex.jpg");
		resman_.LoadResource(Texture, "Cloud", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/gametitle.jpg");
		resman_.LoadResource(Texture, "GameTitle", filename.c_str());
	}

	void Game::SetupScene(void) {
		// Set background color for the scene
		scene_.SetBackgroundColor(viewport_background_color_g);

		//generate the ground for the scene
		Resource *geom = resman_.GetResource("Terrain");
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + "Terrain" + std::string("\"")));
		}

		Resource *mat = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		SceneNode* ground = new SceneNode("Ground", geom, mat);
		scene_.SetRoot(ground);
		//ground->SetScale(glm::vec3(0.5, 1.0, 0.5));
		ground->SetPosition(glm::vec3(0, -100, 200));

		//generate a test enemy and target for him to chase
		Resource *cube = resman_.GetResource("CubePointSet");
		SceneNode *target = new SceneNode("Target", cube, mat);
		target->SetPosition(10, 90, 400);
		//target->SetScale(10, 10, 10);
		ground->AddChild(target);

		Enemy* baddie = new Enemy("Enemy", target, cube, mat);
		baddie->SetPosition(10, 80, 400);
		baddie->SetScale(2, 2, 30);
		ground->AddChild(baddie);

		//this is stored to cheesily draw the helicopter later
		prgm = resman_.GetResource("ObjectMaterial")->GetResource();

		//draw our title screen text on a square (cause i have no idea how to draw text on the screen)
		title = Cube(UI, "TitleScreen", "CubePointSet", "ShinyTextureMaterial", "GameTitle");
		title->SetPosition(0, 0, 765);
		title->Scale(17.3, 13.0, 0.01);
	}

	void Game::MainLoop(void) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// Loop while the user did not close the window
		while (!glfwWindowShouldClose(window_)) {
			if (game_state == TITLE) { //on title screen we do nothing but display the UI
				glClearColor(0.3, 0.1, 0.2, 0.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				title->Draw(&camera_, glm::mat4(1.0));
			}
			else if (game_state == GAME) { //gameplay screen updates and draws the scene
				if (animating_) {
					static double last_time = 0;
					double current_time = glfwGetTime();
					double deltaTime = current_time - last_time;
					if (deltaTime > 0.05) {
						scene_.Update(deltaTime);
						camera_.Update(); //update our camera to keep momentum going with thrusters
						heli_.Update(deltaTime);
						last_time = current_time;

						SceneNode* targ = scene_.GetNode("Target");
						if (temp) {
							targ->Translate(vel);
						}
					}
				} //end if animating_

				// Draw the scene
				scene_.Draw(&camera_);
				heli_.DrawHelicopter(prgm, &camera_); //helicopter is just drawn as UI for now
			}

			glfwSwapBuffers(window_); // Push buffer drawn in the background onto the display
			glfwPollEvents(); // Update other events like input handling
		}
	}

	void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		// Get user data with a pointer to the game class
		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;

		if (game->game_state == TITLE) { //keybinds for title screen
			if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
				game->game_state = GAME;
			}
		}
		else if (game->game_state == GAME) { //keybinds for gameplay
			if (key == GLFW_KEY_KP_9 && action == GLFW_PRESS) {
				Enemy* enemy = ((Enemy*)(game->scene_.GetNode("Enemy")));
				enemy->rotateSpeed = std::max(0.0f, std::min(enemy->rotateSpeed + 0.1f, 1.0f));
				std::cout << enemy->rotateSpeed << std::endl;
			}
			if (key == GLFW_KEY_KP_7 && action == GLFW_PRESS) {
				Enemy* enemy = ((Enemy*)(game->scene_.GetNode("Enemy")));
				enemy->rotateSpeed = std::max(0.0f, std::min(enemy->rotateSpeed - 0.1f, 1.0f));
				std::cout << enemy->rotateSpeed << std::endl;
			}
			if (key == GLFW_KEY_Y && action == GLFW_PRESS) { //reset target position
				game->scene_.GetNode("Target")->SetPosition(10, 90, 400);
			}
			if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS) {
				vel = glm::vec3(0, 1, 0);
			}
			if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS) {
				vel = glm::vec3(0, -1, 0);
			}
			if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS) {
				vel = glm::vec3(1, 0, 0);
			}
			if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS) {
				vel = glm::vec3(-1, 0, 0);
			}
			if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS) {
				vel = glm::vec3(0, 0, -1);
			}
			if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS) {
				vel = glm::vec3(0, 0, 1);
			}
			if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS) {
				vel = glm::vec3(0, 0, 0);
			}
			if (key == GLFW_KEY_T && action == GLFW_PRESS) {
				temp = !temp;
			}
			if (key == GLFW_KEY_V && action == GLFW_PRESS) { //change polygon display modes
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			if (key == GLFW_KEY_B && action == GLFW_PRESS) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { // Quit game if 'ESC' is pressed
				glfwSetWindowShouldClose(window, true);
			}
			if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { // Pause animation if space bar is pressed
				game->animating_ = (game->animating_ == true) ? false : true;
			}

			// Controls to fly the helicopter (just awful right now - work in progress)
			float deviation = 0.5 + ((rand() % 101) / 100); //ranges from 50% to 150% rotation speed
			float rot_factor(deviation * glm::pi<float>() / 180); //deviates a bit to add realism?
			float trans_factor = 0.5;
			if (key == GLFW_KEY_UP) {
				game->camera_.Pitch(rot_factor);
			}
			if (key == GLFW_KEY_DOWN) {
				game->camera_.Pitch(-rot_factor);
			}
			if (key == GLFW_KEY_LEFT) {
				game->camera_.Yaw(rot_factor);
			}
			if (key == GLFW_KEY_RIGHT) {
				game->camera_.Yaw(-rot_factor);
			}
			if (key == GLFW_KEY_S) {
				game->camera_.Roll(-rot_factor);
			}
			if (key == GLFW_KEY_X) {
				game->camera_.Roll(rot_factor);
			}
			if (key == GLFW_KEY_A) { //translation movements are based on an accelerating velocity now
				game->camera_.vel_z += trans_factor;
			}
			if (key == GLFW_KEY_Z) {
				game->camera_.vel_z -= trans_factor;
			}
			if (key == GLFW_KEY_J) {
				game->camera_.vel_x -= trans_factor;
			}
			if (key == GLFW_KEY_L) {
				game->camera_.vel_x += trans_factor;
			}
			if (key == GLFW_KEY_I) {
				game->camera_.vel_y += trans_factor;
			}
			if (key == GLFW_KEY_K) {
				game->camera_.vel_y -= trans_factor;
			}
			if (key == GLFW_KEY_W) { //always good to have a brake handy
				game->camera_.vel_x = 0;
				game->camera_.vel_y = 0;
				game->camera_.vel_z = 0;
			}
			if (key == GLFW_KEY_E && action == GLFW_PRESS) { //fire!
				game->FireLaser();
			}
		} //end elseif game_state
	}

	void Game::ResizeCallback(GLFWwindow* window, int width, int height) {
		// Set up viewport and camera projection based on new window size
		glViewport(0, 0, width, height);
		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;
		game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
		game->heli_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
	}

	Game::~Game() {
		glfwTerminate();
	}

	SceneNode *Game::Cube(int type, std::string entity_name, std::string object_name, std::string material_name, std::string tex_name) {
		// Get resources
		Resource *geom = resman_.GetResource(object_name);
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}
		Resource *mat = resman_.GetResource(material_name);
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}
		Resource *tex = NULL;
		if (tex_name != "") {
			tex = resman_.GetResource(tex_name);
			if (!tex) {
				throw(GameException(std::string("Could not find resource \"") + tex_name + std::string("\"")));
			}
		}

		SceneNode* cube; //generate a cube based on whichever type was passed in
		if (type == LaserBeam) {
			cube = new Laser(entity_name, geom, mat, tex);
			scene_.root_->AddChild(cube);
		}
		else if (type == UI) {
			cube = new SceneNode(entity_name, geom, mat, tex);
		}
		return cube;
	}

	void Game::FireLaser() {
		scene_.Remove("Laser1"); //only one laser can exist at once
		Laser *cube = (Laser*)Cube(LaserBeam, "Laser1", "CubePointSet", "ObjectMaterial");
		cube->SetPosition(camera_.GetPosition() + (camera_.GetForward() * 8.0f) + (camera_.GetUp() * -0.5f)); //fires outward from the heli's position
		cube->SetOrientation(camera_.GetOrientation());
		cube->SetDir(camera_.GetForward());
		cube->SetScale(glm::vec3(0.2, 0.2, 0.2));

		std::string collide = RaySphere(camera_.GetForward(), cube->GetPosition()); //terribly inaccurate hitscan collision
		std::cout << collide << std::endl;
		if (collide != "none") {
			scene_.Remove(collide); //blow up the object we collided with
		}
	}

	std::string Game::RaySphere(glm::vec3 raydir, glm::vec3 raypos) {
		for (int i = 0; i < scene_.root_->children_.size() - 1; i++) { //check against all objects
			glm::vec3 d = scene_.root_->children_[i]->GetPosition() - raypos + scene_.root_->GetPosition();
			float projectD = glm::dot(d, raydir);
			float d2 = glm::dot(d, d) - projectD * projectD;
			if (d2 > 1.0) continue; //can't collide with this sphere, move on
			float d3 = sqrt(1.0 - d2);
			//possible solutions
			float s1 = projectD - d3;
			float s2 = projectD + d3;
			if (s1 >= 0 || s2 >= 0) { //we found one, return the name
				return scene_.root_->children_[i]->GetName();
			}
		}
		return "none"; //didn't find anything
	}

} // namespace game
