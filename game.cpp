#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include "game.h"
#include "bin/path_config.h"

namespace game {
	// Main window settings
	const std::string window_title_g = "Hippity Hoppity";
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
		// Run all initialization steps
		InitWindow();
		InitView();
		InitEventHandlers();

		// Set variables
		player_vel = glm::vec3(0, 0, 0);
		animating_ = true;
		game_state = TITLE; //start on title screen
		hp = 100.0;
		sun = true; //the sun rises
		tpCam = false; //start in first person
		turning = NONE;
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
		resman_.CreateTorus("TorusMesh");
		resman_.CreateCylinder("CylinderMesh");
		resman_.CreateGround("Terrain"); // load terrain

		// Load non textured materials
		std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
		resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/dark");
		resman_.LoadResource(Material, "DarkMaterial", filename.c_str());

		//load texture materials
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_texture");
		resman_.LoadResource(Material, "ShinyTextureMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/gametitle.jpg");
		resman_.LoadResource(Texture, "GameTitle", filename.c_str());
		
		// load textures for models
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/bunBodTex.png");
		resman_.LoadResource(Texture, "BunnyBodTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/bunEarTex.png");
		resman_.LoadResource(Texture, "BunnyEarTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/catTex.png");
		resman_.LoadResource(Texture, "CatTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/pupperTex.png");
		resman_.LoadResource(Texture, "DogTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/fireworkTex.png");
		resman_.LoadResource(Texture, "FireworkTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/furrTex.png");
		resman_.LoadResource(Texture, "FurrBallTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/gunTex.png");
		resman_.LoadResource(Texture, "GunTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/moleTex.png");
		resman_.LoadResource(Texture, "MoleTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/propTex.png");
		resman_.LoadResource(Texture, "PropellerTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/tennisTex.png");
		resman_.LoadResource(Texture, "TennisBallTex", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture/carrotTex.png");
		resman_.LoadResource(Texture, "CarrotTex", filename.c_str());
		
		// Load Meshes
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/bunny_body.obj");
		resman_.LoadResource(Mesh, "BunnyBodMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/carrotMesh.obj");
		resman_.LoadResource(Mesh, "CarrotMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/bunny_ears.obj");
		resman_.LoadResource(Mesh, "BunnyEarMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/catMesh.obj");
		resman_.LoadResource(Mesh, "CatMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/fireworkMesh.obj");
		resman_.LoadResource(Mesh, "FireworkMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/furrBallMesh.obj");
		resman_.LoadResource(Mesh, "FurrBallMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/gunMesh.obj");
		resman_.LoadResource(Mesh, "GunMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/moleMesh.obj");
		resman_.LoadResource(Mesh, "MoleMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/propMesh.obj");
		resman_.LoadResource(Mesh, "PropellerMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/pupperMesh.obj");
		resman_.LoadResource(Mesh, "DogMesh", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/mesh/tennisBallMesh.obj");
		resman_.LoadResource(Mesh, "TennisBallMesh", filename.c_str());

		//screen space effect
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/screen_hp");
		resman_.LoadResource(Material, "BlueMaterial", filename.c_str());

		//particle effect material for bomb
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/particle");
		resman_.LoadResource(Material, "ParticleMaterial", filename.c_str());
		//material for tracer
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/line");
		resman_.LoadResource(Material, "LineMaterial", filename.c_str());

		// Create particles for firework weapon and tracer
		resman_.CreateSphereParticles("SphereParticles");
		resman_.CreateLineParticles("LineParticles");

		// Load texture to be applied to particles
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/firework.png");
		resman_.LoadResource(Texture, "Firework", filename.c_str());

		scene_.SetupDrawToTexture();
	}

	void Game::SetupScene(void) {
		scene_.world_bl_corner = glm::vec3(280, 0, 280);
		scene_.world_tr_corner = glm::vec3(700, 200, 700);

		scene_.SetResourceManager(&resman_);

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
		Resource *tmat = resman_.GetResource("ShinyTextureMaterial");
		if (!tmat) {
			throw(GameException(std::string("Could not find resource \"") + "ShinyTextureMaterial" + std::string("\"")));
		}
		Resource *dark = resman_.GetResource("DarkMaterial");
		if (!dark) {
			throw(GameException(std::string("Could not find resource \"") + "DarkMaterial" + std::string("\"")));
		}

		SceneNode* ground = new SceneNode("Ground", geom, dark);
		scene_.SetRoot(ground);
		ground->SetPosition(glm::vec3(0, -100, 200));

		Resource *sphere = resman_.GetResource("SimpleSphereMesh");
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *torus = resman_.GetResource("TorusMesh");
		Resource *cyl = resman_.GetResource("CylinderMesh");

		SceneNode* player = heli_.initHeli(&resman_, &scene_);
		player->SetPosition(200, 100, 200);
		ground->AddChild(player);

		SceneNode *target = new SceneNode("Target", cube, mat, NULL, true);
		target->SetPosition(190, 30.5, 200);
		target->SetScale(1, 1, 1);
		ground->AddChild(target);

		scene_.root_->AddChild(SpawnCat());
		scene_.root_->AddChild(SpawnMole());
		scene_.root_->AddChild(SpawnDog());

		for (int i = 0; i < 3; i++)
		{
			float v = (double)rand() / (double)RAND_MAX;
			if (v > 0.6) { 
				scene_.root_->AddChild(SpawnMole());
			}
			else if (v > 0.3) {
				scene_.root_->AddChild(SpawnCat());
			}
			else {
				scene_.root_->AddChild(SpawnDog());
			}
		}

		SceneNode* ground_bound = new SceneNode("Ground_Box", cube, mat);
		ground_bound->setCollidable(true);
		ground_bound->SetScale(1000, 300, 1000);
		ground_bound->SetPosition(500, -153, 500);
		ground->AddChild(ground_bound);

		SceneNode* tree;

		for (int i = 0; i < 15; i++)
		{
			ground->AddChild(SpawnTree());
		}

		target->SetPosition(0, 100, 510);

		//this is stored to cheesily draw the helicopter later
		prgm = resman_.GetResource("ObjectMaterial")->GetResource();

		SceneNode* test_sun = new SceneNode("Sun", sphere, mat); //the glorious lightsource
		test_sun->SetPosition(160.0, 150.0, 300.0);
		ground->AddChild(test_sun);

		//draw our title screen text on a square (cause i have no idea how to draw text on the screen)
		title = Cube(UI, "TitleScreen", "CubePointSet", "ShinyTextureMaterial", glm::vec3(0.0, 0.0, 0.0), 0.0, "GameTitle");
		title->SetPosition(0, 0, 765);
		title->Scale(17.3, 13.0, 0.01);
	}

	void Game::MainLoop(void) {
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);

		// Loop while the user did not close the window
		while (!glfwWindowShouldClose(window_)) {
			if (game_state == TITLE) { //on title screen we do nothing but display the UI
				glClearColor(0.3, 0.1, 0.2, 0.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				title->Draw(&camera_, glm::mat4(1.0), true);
			}
			else if (game_state == GAME) { //gameplay screen updates and draws the scene
				if (animating_) {
					static double last_time = 0;
					double current_time = glfwGetTime();
					double deltaTime = current_time - last_time;
					//time_since_spawn += deltaTime;

					if (deltaTime > 0.01) {
						scene_.Update(deltaTime);
						camera_.Update(); //update our camera to keep momentum going with thrusters

					/*	SceneNode* player = scene_.GetNode("Player");


						glm::vec3 forward = SceneNode::default_forward * player->GetOrientation();
						player->Translate(player_vel * forward);
						camera_.SetPosition(scene_.GetNode("Player")->GetAbsolutePosition() + camera_.GetForward());
						*/

						heli_.Update(deltaTime);
						last_time = current_time;

						//handle turning here
						float rot_factor(1.5 * glm::pi<float>() / 180);
						if (turning == LEFT) {
							camera_.Yaw(rot_factor);
						}
						else if (turning == RIGHT) {
							camera_.Yaw(-rot_factor);
						}
						else if (turning == UP) {
							camera_.Pitch(rot_factor);
						}
						else if (turning == DOWN) {
							camera_.Pitch(-rot_factor);
						}

						if (time_since_spawn > 20) {
							if (rand() > RAND_MAX / 2) {
								scene_.root_->AddChild(SpawnMole());
							}
							else {
								scene_.root_->AddChild(SpawnDog());
							}

							time_since_spawn = 0;
						}

						SceneNode* targ = scene_.GetNode("Target");
						if (temp && targ != NULL) {
							targ->SetPosition(camera_.GetPosition() - scene_.GetNode("Ground")->GetPosition() - camera_.GetForward());
						}
					}
				} //end if animating_

				// Draw the scene
				if (tpCam) { //third person
					scene_.Draw(&camera_);
				//	heli_.DrawHelicopter(prgm, &camera_); //helicopter is just drawn as UI for now
				}
				else { //first person
					scene_.DrawToTexture(&camera_, sun);
					scene_.DisplayTexture(resman_.GetResource("BlueMaterial")->GetResource(), hp);
				}
				scene_.CheckCollisions();
			} //end of GAME gamestate
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
			float trans_factor = 1.0;
			if (key == GLFW_KEY_KP_9 && action == GLFW_PRESS) {
				Enemy* enemy = ((Enemy*)(game->scene_.GetNode("Enemy0")));
				enemy->setRotateSpeed(std::max(0.0f, std::min(enemy->getRotateSpeed() + 0.1f, 1.0f)));
				std::cout << enemy->getRotateSpeed() << std::endl;
			}
			if (key == GLFW_KEY_KP_7 && action == GLFW_PRESS) {
				Enemy* enemy = ((Enemy*)(game->scene_.GetNode("Enemy0")));
				enemy->setRotateSpeed(std::max(0.0f, std::min(enemy->getRotateSpeed() - 0.1f, 1.0f)));
				std::cout << enemy->getRotateSpeed() << std::endl;
			}
			if (key == GLFW_KEY_Y && action == GLFW_PRESS) { //tell me target position
				glm::vec3 pos = game->scene_.GetNode("Target")->GetPosition();

				std::cout << "Targ @ [" << pos.x << ", " << pos.y << ", " << pos.z << "]" << std::endl;
			}
			if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS) {
				game->scene_.GetNode("Target")->Translate(0, 0.1, 0);
				vel = glm::vec3(0, 1, 0);
			}
			if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS) {
				game->scene_.GetNode("Target")->Translate(0, -1, 0);
				vel = glm::vec3(0, -1, 0);
			}
			if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS) {
				game->scene_.GetNode("Target")->Translate(1, 0, 0);
				vel = glm::vec3(1, 0, 0);
			}
			if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS) {
				game->scene_.GetNode("Target")->Translate(-1, 0, 0);
				vel = glm::vec3(-1, 0, 0);
			}
			if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS) {
				game->scene_.GetNode("Target")->Translate(0, 0, -1);
				vel = glm::vec3(0, 0, -1);
			}
			if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS) {
				game->scene_.GetNode("Target")->Translate(0, 0, 1);
				vel = glm::vec3(0, 0, 1);
			}
			if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS) {
				//game->animating_ = !game->animating_;
			}
			if (key == GLFW_KEY_T && action == GLFW_PRESS) {
				temp = true;
				glm::vec3 forward = game->camera_.GetForward();
				glm::vec3 origin = game->camera_.GetPosition();

				Resource *cube = game->resman_.GetResource("CubePointSet");
				Resource *mat = game->resman_.GetResource("ObjectMaterial");
				if (!mat) {
					throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
				}

				Projectile* p = new Projectile("Player", game->camera_.GetPosition() - game->scene_.GetNode("Ground")->GetAbsolutePosition(), forward*1.0f, glm::vec3(0, -0.5, 0), INFINITY, cube, mat);
				p->SetScale(0.5, 0.5, 2);
				//game->scene_.AddProjectile(p);

				//game->scene_.GetNode("Ground")->AddChild(game->SpawnDog());

				//std::vector<std::pair<SceneNode*, glm::vec2*>> hit = game->scene_.CheckRayCollisions(Ray(origin, forward));

				/*for (int i = 0; i < hit.size(); i++) {
				if (hit[i].first->GetName() != "Target") {
				hit[i].first->takeDamage(9999);
				}
				}*/
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
			if (key == GLFW_KEY_TAB && action == GLFW_PRESS) { // switch camera if TAB is pressed
				if (game->tpCam) { //when entering first person, position the camera forward
					game->camera_.SetPosition(game->camera_.GetPosition() + game->camera_.GetForward() * 10.0f);
				}
				else { //when entering third person, position the camera behind the player
					game->camera_.SetPosition(game->camera_.GetPosition() - game->camera_.GetForward() * 10.0f);
				}
				game->tpCam = !game->tpCam;
			}

			//MOVEMENT HERE
			if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { // spacebar hovers upwards
				//game->player_vel.y = trans_factor;
				game->camera_.vel_y = trans_factor;
			}
			if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
				//game->player_vel.y = 0.0;
				game->camera_.vel_y = 0.0;
			}
			if (key == GLFW_KEY_X && action == GLFW_PRESS) { // x hovers downwards
				//game->player_vel.y = -trans_factor;
				game->camera_.vel_y = -trans_factor;
			}
			if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
				//game->player_vel.y= 0.0;
				game->camera_.vel_y = 0;
			}
			if (key == GLFW_KEY_S && action == GLFW_PRESS) { // s moves backwards
				//game->player_vel.z = -trans_factor;
				game->camera_.vel_z = -trans_factor;
			}
			if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
				//game->player_vel.z = 0.0;
				game->camera_.vel_z = 0;
			}
			if (key == GLFW_KEY_W && action == GLFW_PRESS) { // w moves forwards
				//game->player_vel.z = trans_factor;
				game->camera_.vel_z = trans_factor;
			}
			if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
				//game->player_vel.z = 0.0;
				game->camera_.vel_z = 0;
			}
			if (key == GLFW_KEY_Q && action == GLFW_PRESS) { // Q strafes left
				//game->player_vel.x = -trans_factor;
				game->camera_.vel_x = -trans_factor;
			}
			if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
				//game->player_vel.x = 0.0;
				game->camera_.vel_x = 0;
			}
			if (key == GLFW_KEY_E && action == GLFW_PRESS) { // E strafes right
				//game->player_vel.x = trans_factor;
				game->camera_.vel_x = trans_factor;
			}
			if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
				//game->player_vel.x = 0.0;
				game->camera_.vel_x = 0;
			}

			//TURNING HERE
			float rot_factor(1.5 * glm::pi<float>() / 180);
			if (key == GLFW_KEY_A && action == GLFW_PRESS) { // a turns left
				game->turning = LEFT;
				if (game->tpCam) { game->camera_.vel_x = trans_factor; } //third person turning L
			}
			if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
				game->turning = NONE;
				if (game->tpCam) { game->camera_.vel_x = 0.0; }
			}
			if (key == GLFW_KEY_D && action == GLFW_PRESS) { // d turns right
				game->turning = RIGHT;
				if (game->tpCam) { game->camera_.vel_x = -trans_factor; } //third person turning R
			}
			if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
				game->turning = NONE;
				if (game->tpCam) { game->camera_.vel_x = 0.0; }
			}
			if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
				game->turning = UP;
			}
			if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
				game->turning = NONE;
			}
			if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
				game->turning = DOWN;
			}
			if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
				game->turning = NONE;
			}
			if (key == GLFW_KEY_LEFT) {
				game->camera_.Roll(-rot_factor);
			}
			if (key == GLFW_KEY_RIGHT) {
				game->camera_.Roll(rot_factor);
			}

			if (key == GLFW_KEY_R && action == GLFW_PRESS) { //fires a bomb with particle effects
				game->FireBomb();
			}
			if (key == GLFW_KEY_C && action == GLFW_PRESS) { //fires a tracer with particle effects
				game->FireTracer();
			}
			if (key == GLFW_KEY_F && action == GLFW_PRESS) { //fire a laser
				game->FireLaser();
			}
			if (key == GLFW_KEY_H && action == GLFW_PRESS) { // H increases hp
				game->hp += 5;
			}
			if (key == GLFW_KEY_G && action == GLFW_PRESS) { // G lowers hp
				game->hp -= 5;
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

	SceneNode *Game::Cube(int type, std::string entity_name, std::string object_name, std::string material_name,
		glm::vec3 rgb, double ttl, std::string tex_name) {
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
		if (!tex_name.empty()) {
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
		else if (type == BombType) {
			cube = new Bomb(entity_name, geom, mat, ttl, rgb, tex); //help
			scene_.root_->AddChild(cube);
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
		if (collide == "Sun") {
			sun = false; //we killed the sun
		}
		if (collide != "none") {
			scene_.Remove(collide); //blow up the object we collided with
		}
	}

	void Game::FireBomb() {
		//139,69,19
		float ttlr = 3.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (4.5 - 3.0)));
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

		game::Bomb *particles = (Bomb*)Cube(BombType, "ParticleInstance" + std::to_string(scene_.exploCount++), "SphereParticles", "ParticleMaterial",
			glm::vec3(r, g, b), 4.0, "Firework");
		particles->SetPosition(camera_.GetPosition() + (camera_.GetForward() * 12.0f) + (camera_.GetUp() * -0.5f));
		particles->SetScale(glm::vec3(0.4, 0.4, 0.4)); //increasing the scale makes the explosion more dense
	}

	void Game::FireTracer() {
		float ttlr = 3.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (4.5 - 3.0)));
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

		game::Bomb *particles = (Bomb*)Cube(BombType, "TracerInstance" + scene_.exploCount++, "LineParticles", "LineMaterial",
			glm::vec3(r, g, b), 6.0, "Firework");
		particles->SetScale(glm::vec3(0.02, 0.02, 1000.0)); //1000 is ~far away~
		particles->SetPosition(camera_.GetPosition() + (camera_.GetForward() * 200.0f) + (camera_.GetUp() * -0.3f)); //200.0f starts the line on the playerish
		particles->SetOrientation(camera_.GetOrientation());
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

	SceneNode* Game::CreateMole() {
		// Create a Mole SceneNode tree

		Resource *fireworkMesh = resman_.GetResource("FireworkMesh");
		Resource *fireworkTex = resman_.GetResource("FireworkTex");
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *moleMesh = resman_.GetResource("MoleMesh");
		Resource *moleTex = resman_.GetResource("MoleTex");

		Resource *mat = resman_.GetResource("ShinyTextureMaterial");
		Resource *gunMesh = resman_.GetResource("GunMesh");
		Resource *gunTex = resman_.GetResource("GunTex");
		Resource *ob = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "TextureMat" + std::string("\"")));
		}

		std::string name = "Enemy" + std::to_string(EnemyID++);
		numEnemies++;


		SceneNode* n = new SceneNode(name, NULL, NULL, NULL);
		//n->setMovementSpeed(0);
		//n->setRotateSpeed(0);

		Mole* body = new Mole(name + "_body", scene_.GetNode("Target"), moleMesh, ob, moleTex);
		body->setCollidable(true);
		body->SetPosition(0, 0.5, 0);
	
		SceneNode* gun = new SceneNode(name + "_gun", gunMesh, ob, gunTex);
		gun->SetPosition(1.0, 0.5, 1.5);
		gun->setCollidable(true);
		gun->SetScale(0.5, 0.5, 5.0);

		body->setProjectileGeometry(fireworkMesh);
		body->setProjectileTexture(fireworkTex);
		body->setProjectileMaterial(ob);

		n->AddChild(body);
		body->AddChild(gun);

		return n;
	}

	SceneNode* Game::CreateDog() {
		
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *dogMesh = resman_.GetResource("DogMesh");
		Resource *dogTex = resman_.GetResource("DogTex");

		Resource *ballMesh = resman_.GetResource("TennisBallMesh");
		Resource *ballTex = resman_.GetResource("TennisBallTex");

		Resource *mat = resman_.GetResource("ShinyTextureMaterial");
		Resource *ob = resman_.GetResource("ObjectMaterial");
		
		Resource *gunMesh = resman_.GetResource("GunMesh");
		Resource *gunTex = resman_.GetResource("GunTex");

		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Enemy" + std::to_string(EnemyID++);
		numEnemies++;
		SceneNode* n = new SceneNode(name, NULL, NULL, NULL);

		SceneNode* turret = new SceneNode(name + "_turret", gunMesh, mat, gunTex);
		turret->setCollidable(true);
		turret->SetScale(0.5, 0.5, 4.0);
		turret->SetPosition(0, 0.75, 0);
		//turret->setMovementSpeed(0);

		Doggy* dog = new Doggy(name + "_body", scene_.GetNode("Target"), dogMesh, mat, dogTex);
		dog->SetScale(2.0, 1.0, 6.0);
		dog->Translate(0, 1, 0);
		dog->setCollidable(true);
		dog->setTurret(turret);
		dog->setProjectileGeometry(ballMesh);
		dog->setProjectileTexture(ballTex);
		dog->setProjectileMaterial(mat);

		n->AddChild(turret);
		n->AddChild(dog);
		//n->setAttackingComponent(dog);

		return n;
	}

/*	SceneNode* Game::CreateCat() {
		
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *catMesh = resman_.GetResource("CatMesh");
		Resource *catTex = resman_.GetResource("CatTex");
		
		Resource *propMesh = resman_.GetResource("PropellerMesh");
		Resource *propTex = resman_.GetResource("PropellerTex");

		Resource *mat = resman_.GetResource("ShinyTextureMaterial");
		Resource *ob = resman_.GetResource("ObjectMaterial");
		
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Enemy" + std::to_string(EnemyID++);
		numEnemies++;
		SceneNode* n = new SceneNode(name, NULL, NULL, NULL);

		SceneNode* prop = new SceneNode(name + "_prop", propMesh, ob, propTex);
		turret->setCollidable(true);
		turret->SetScale(0.5, 0.5, 4.0);
		turret->SetPosition(0, 0.75, 0);
		//turret->setMovementSpeed(0);

		Doggy* cat = new Doggy(name + "_body", scene_.GetNode("Target"), catMesh, mat, catTex);
		cat->SetScale(2.0, 1.0, 6.0);
		cat->setCollidable(true);
		cat->setTurret(turret);
		cat->setProjectileGeometry(sphere);
		cat->setProjectileMaterial(mat);

		n->AddChild(turret);
		n->AddChild(cat);
		//n->setAttackingComponent(cat);

		return n;
	}
	*/
	
	SceneNode* Game::CreateTree() {
		Resource *cyl = resman_.GetResource("CylinderMesh");
		Resource *sphere = resman_.GetResource("SimpleSphereMesh");

		Resource *mat = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Tree" + std::to_string(TreeID++);

		int height = rand() % 7 + 5;
		float thicc = height / 10.0f + (((double)rand() / (double)RAND_MAX) - 0.5);
		//fmax(((double)rand() / (double)RAND_MAX) * 2, 0.3) + height/50.0f;

		SceneNode* trunk = new SceneNode(name, cyl, mat);
		trunk->setCollidable(true);
		trunk->SetScale(thicc, height, thicc);

		SceneNode* top = new SceneNode(name + "_leaves", sphere, mat);
		top->setCollidable(false);
		top->SetPosition(0, height / 2.0, 0);
		top->SetScale(thicc * 2, thicc * 2, thicc * 2);

		trunk->AddChild(top);

		return trunk;
	}
	
	SceneNode* Game::CreateCat() {
		Resource *sphere = resman_.GetResource("SimpleSphereMesh");
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *mat = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Enemy" + std::to_string(EnemyID++);

		Cat* c = new Cat(name, scene_.GetNode("Target"), cube, mat, NULL);
		c->setCollidable(true);
		c->SetScale(1.0, 2.0, 1.0);

		c->setProjectileGeometry(sphere);
		c->setProjectileMaterial(mat);

		return c;
	}

	SceneNode* Game::SpawnMole() {
		// create a mole
		SceneNode* m = CreateMole();

		// spawn it somewhere in the world
		glm::vec3 v = scene_.GetRandomBoundedPosition();

		m->SetPosition(v.x, 0, v.z);

		return m;
	}

	SceneNode* Game::SpawnDog() {
		SceneNode* d = CreateDog();

		glm::vec3 v = scene_.GetRandomBoundedPosition();

		d->SetPosition(v.x, 0, v.z);

		return d;
	}
	
/*	SceneNode* Game::SpawnCat() {
		SceneNode* d = CreateCat();

		glm::vec3 v = scene_.GetRandomBoundedPosition();

		d->SetPosition(v.x, 0, v.z);

		return d;
	}
*/
	SceneNode* Game::SpawnTree() {
		// create a tree
		SceneNode* t = CreateTree();

		// spawn it somewhere in the world
		glm::vec3 v = scene_.GetRandomBoundedPosition();

		t->SetPosition(v.x, t->GetScale().y/2, v.z);

		return t;
	}

	SceneNode* Game::SpawnCat() {
		SceneNode* cat = CreateCat();
		cat->SetPosition(scene_.GetRandomBoundedPosition());

		return cat;
	}

} // namespace game
