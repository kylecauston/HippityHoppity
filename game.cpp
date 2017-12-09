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
		player_vel = glm::vec3(0, 0, 0);
		animating_ = true;
		game_state = TITLE;
		hp = 100.0;
		sun = true;
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

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/cloudtex.jpg");
		resman_.LoadResource(Texture, "Cloud", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/gametitle.jpg");
		resman_.LoadResource(Texture, "GameTitle", filename.c_str());

		//screen space effect
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/screen_hp");
		resman_.LoadResource(Material, "BlueMaterial", filename.c_str());

		scene_.SetupDrawToTexture();
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
		Resource *tmat = resman_.GetResource("ShinyTextureMaterial");
		if (!tmat) {
			throw(GameException(std::string("Could not find resource \"") + "ShinyTextureMaterial" + std::string("\"")));
		}
		Resource *tex = resman_.GetResource("Cloud");
		if (!tex) {
			throw(GameException(std::string("Could not find resource \"") + "Cloud" + std::string("\"")));
		}
		Resource *dark = resman_.GetResource("DarkMaterial");
		if (!dark) {
			throw(GameException(std::string("Could not find resource \"") + "DarkMaterial" + std::string("\"")));
		}

		SceneNode* ground = new SceneNode("Ground", geom, dark);
		scene_.SetRoot(ground);
		//ground->SetScale(glm::vec3(0.5, 1.0, 0.5));
		ground->SetPosition(glm::vec3(0, -100, 200));

		//generate a test enemy and target for him to chase

		Resource *sphere = resman_.GetResource("SimpleSphereMesh");
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *torus = resman_.GetResource("TorusMesh");
		Resource *cyl= resman_.GetResource("CylinderMesh");

		SceneNode* player = new SceneNode("Player", cube, mat, NULL, true);
		player->SetPosition(200, 100, 200);
		ground->AddChild(player);

		SceneNode *target = new SceneNode("Target", cube, mat, NULL, true);
		target->SetPosition(190, 30.5, 200);
		target->SetScale(1, 1, 1);
		ground->AddChild(target);

		SceneNode* mole = CreateMole();
		mole->SetPosition(200, 26, 200);
		ground->AddChild(mole);

		SceneNode* box = new SceneNode("testing box", sphere, mat);
		box->setCollidable(true);
		box->SetScale(10, 10, 10);
		box->SetPosition(0, 100, 480);
		//ground->AddChild(box);

		SceneNode* tester = CreateDog();
		tester->SetPosition(0, 80, 510);
		ground->AddChild(tester);

		SceneNode* tree;

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				tree = CreateTree();
				tree->SetPosition(i * 10, 80 + tree->GetScale().y / 2, 500-j*10);
				//ground->AddChild(tree);
			}
		}

		target->SetPosition(0, 100, 510);


		//this is stored to cheesily draw the helicopter later
		prgm = resman_.GetResource("ObjectMaterial")->GetResource();

		SceneNode* test_sun = new SceneNode("Sun", sphere, mat); //the glorious lightsource
		test_sun->SetPosition(160.0, 150.0, 300.0);
		ground->AddChild(test_sun);

		//draw our title screen text on a square (cause i have no idea how to draw text on the screen)
		title = Cube(UI, "TitleScreen", "CubePointSet", "ShinyTextureMaterial", "GameTitle");
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
					if (deltaTime > 0.05) {
						scene_.Update(deltaTime);
						camera_.Update(); //update our camera to keep momentum going with thrusters
						
					/*	SceneNode* player = scene_.GetNode("Player");
						

						glm::vec3 forward = SceneNode::default_forward * player->GetOrientation();
						player->Translate(player_vel * forward);
						camera_.SetPosition(scene_.GetNode("Player")->GetAbsolutePosition() + camera_.GetForward());
						*/

						heli_.Update(deltaTime);
						last_time = current_time;

						//handle turning here LUL
						float rot_factor(1.5 * glm::pi<float>() / 180);
						if (turning == 1) {
							camera_.Yaw(rot_factor);
						}
						else if (turning == 2) {
							camera_.Yaw(-rot_factor);
						}
						else if (turning == 3) {
							camera_.Pitch(rot_factor);
						}
						else if (turning == 4) {
							camera_.Pitch(-rot_factor);
						}

						SceneNode* targ = scene_.GetNode("Target");
						if (temp && targ != NULL) {
							targ->SetPosition(camera_.GetPosition() - scene_.GetNode("Ground")->GetPosition() - camera_.GetForward());
						}
					}
				} //end if animating_

				// Draw the scene
				//scene_.Draw(&camera_);
				scene_.DrawToTexture(&camera_, sun);
				scene_.DisplayTexture(resman_.GetResource("BlueMaterial")->GetResource(), hp);
				//heli_.DrawHelicopter(prgm, &camera_); //helicopter is just drawn as UI for now

				scene_.CheckCollisions();
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
			float trans_factor = 3.0;
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

				std::cout << "Targ @ [" << pos.x << ", "  << pos.y << ", " << pos.z << "]" << std::endl;
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
				//temp = true;
				glm::vec3 forward = game->camera_.GetForward();
				glm::vec3 origin = game->camera_.GetPosition();	
				
				Resource *cube = game->resman_.GetResource("CubePointSet");
				Resource *mat = game->resman_.GetResource("ObjectMaterial");
				if (!mat) {
					throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
				}

				Projectile* p = new Projectile("Player", game->camera_.GetPosition() - game->scene_.GetNode("Ground")->GetAbsolutePosition(), forward*1.0f, glm::vec3(0, -0.5, 0), INFINITY, cube, mat);
				p->SetScale(0.5, 0.5, 2);
				game->scene_.AddProjectile(p);

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
				//game->camera_.Yaw(rot_factor);
				game->turning = 1;
			}
			if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
				game->turning = 0;
			}
			if (key == GLFW_KEY_D && action == GLFW_PRESS) { // d turns right
				//game->camera_.Yaw(-rot_factor);
				game->turning = 2;
			}
			if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
				game->turning = 0;
			}
			if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
				game->turning = 3;
			}
			if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
				game->turning = 0;
			}
			if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
				game->turning = 4;
			}
			if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
				game->turning = 0;
			}
			if (key == GLFW_KEY_LEFT) {
				game->camera_.Roll(-rot_factor);
			}
			if (key == GLFW_KEY_RIGHT) {
				game->camera_.Roll(rot_factor);
			}

			if (key == GLFW_KEY_R) { //always good to have a brake handy
				game->camera_.vel_x = 0;
				game->camera_.vel_y = 0;
				game->camera_.vel_z = 0;
			}
			if (key == GLFW_KEY_F && action == GLFW_PRESS) { //fire!
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
			sun = false;
		}
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

	SceneNode* Game::CreateMole() {
		// Create a Mole SceneNode tree

		Resource *sphere = resman_.GetResource("SimpleSphereMesh");
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *torus = resman_.GetResource("TorusMesh");

		Resource *mat = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Enemy" + std::to_string(EnemyID++);
		numEnemies++;


		SceneNode* n = new SceneNode(name, NULL, NULL, NULL);
		//n->setMovementSpeed(0);
		//n->setRotateSpeed(0);
		
		SceneNode* dirt = new SceneNode(name + "_dirt", torus, mat);
		dirt->setCollidable(false);
		dirt->SetScale(2.0, 2.0, 2.0);
		dirt->SetOrientation(glm::angleAxis(glm::pi<float>() / 2, glm::vec3(1.0, 0.0, 0.0)));

		Mole* body = new Mole(name + "_body", scene_.GetNode("Target"), sphere, mat);
		body->setCollidable(true);
		body->SetPosition(0, 0.5, 0);
	
		SceneNode* gun = new SceneNode(name + "_gun", cube, mat);
		gun->SetPosition(1.0, 0.5, 1.5);
		gun->setCollidable(true);
		gun->SetScale(0.5, 0.5, 5.0);

		body->setProjectileGeometry(sphere);
		body->setProjectileMaterial(mat);
		
		n->AddChild(dirt);
		n->AddChild(body);
		//n->setAttackingComponent(body);
		body->AddChild(gun);

		return n;
	}

	SceneNode* Game::CreateDog() {
		Resource *cube = resman_.GetResource("CubePointSet");
		Resource *sphere = resman_.GetResource("SimpleSphereMesh");


		Resource *mat = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Enemy" + std::to_string(EnemyID++);
		numEnemies++;
		SceneNode* n = new SceneNode(name, NULL, NULL, NULL);

		SceneNode* turret = new SceneNode(name + "_turret", cube, mat);
		turret->setCollidable(true);
		turret->SetScale(0.5, 0.5, 4.0);
		turret->SetPosition(0, 0.75, 0);
		//turret->setMovementSpeed(0);

		Doggy* dog = new Doggy(name + "_body", scene_.GetNode("Target"), cube, mat);
		dog->SetScale(2.0, 1.0, 6.0);
		dog->setCollidable(true);
		dog->setTurret(turret);
		dog->setProjectileGeometry(sphere);
		dog->setProjectileMaterial(mat);

		n->AddChild(turret);
		n->AddChild(dog);
		//n->setAttackingComponent(dog);

		return n;
	}

	SceneNode* Game::CreateTree() {
		Resource *cyl = resman_.GetResource("CylinderMesh");
		Resource *sphere = resman_.GetResource("SimpleSphereMesh");

		Resource *mat = resman_.GetResource("ObjectMaterial");
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + "ObjectMaterial" + std::string("\"")));
		}

		std::string name = "Tree" + std::to_string(TreeID++);

		int height = rand() % 7 + 5;
		float thicc = fmax(((double)rand() / (double)RAND_MAX) * 2, 0.3);

		SceneNode* trunk = new SceneNode(name, cyl, mat);
		trunk->setCollidable(true);
		trunk->SetScale(thicc, height, thicc);

		SceneNode* top = new SceneNode(name + "_leaves", sphere, mat);
		top->setCollidable(false);
		top->SetPosition(0, height/2.0 ,0);
		top->SetScale(thicc * 1.5, thicc * 1.5, thicc * 1.5);
		
		trunk->AddChild(top);

		return trunk;
	}

} // namespace game
