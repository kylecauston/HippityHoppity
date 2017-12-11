cmake_minimum_required(VERSION 2.6)

# Name of project
project(HippityHoppity)

# Specify project files: header files and source files
set(HDRS
    aabb.h attack_node.h bomb.h camera.h cat.h collidable.h collision_manager.h defs.h doggy.h enemy.h game.h helicopter.h hitbox.h hitscan.h laser.h mole.h projectile.h ray.h resource.h resource_manager.h scene_graph.h scene_node.h
)
 
set(SRCS
    aabb.cpp attack_node.cpp bomb.cpp camera.cpp cat.cpp collidable.cpp collision_manager.cpp doggy.cpp enemy.cpp game.cpp helicopter.cpp hitbox.cpp hitscan.cpp laser.cpp main.cpp mole.cpp projectile.cpp ray.cpp resource.cpp resource_manager.cpp scene_graph.cpp scene_node.cpp dark_fp.glsl dark_vp.glsl line_fp.glsl line_gp.glsl line_vp.glsl material_fp.glsl material_vp.glsl particle_fp.glsl particle_gp.glsl particle_vp.glsl screen_hp_fp.glsl screen_hp_vp.glsl shiny_texture_fp.glsl shiny_texture_vp.glsl
)

# Add path name to configuration file
configure_file(path_config.h.in path_config.h)

# Add executable based on the source files
add_executable(HippityHoppity ${HDRS} ${SRCS})

# Require OpenGL library
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(HippityHoppity ${OPENGL_gl_LIBRARY})

# Other libraries needed
set(LIBRARY_PATH "" CACHE PATH "Folder with GLEW, GLFW, GLM, and SOIL libraries")
include_directories(${LIBRARY_PATH}/include)
if(NOT WIN32)
    find_library(GLEW_LIBRARY GLEW)
    find_library(GLFW_LIBRARY glfw)
    find_library(SOIL_LIBRARY SOIL)
elseif(WIN32)
    find_library(GLEW_LIBRARY glew32s HINTS ${LIBRARY_PATH}/lib)
    find_library(GLFW_LIBRARY glfw3 HINTS ${LIBRARY_PATH}/lib)
    find_library(SOIL_LIBRARY SOIL HINTS ${LIBRARY_PATH}/lib)
endif(NOT WIN32)
target_link_libraries(HippityHoppity ${GLEW_LIBRARY})
target_link_libraries(HippityHoppity ${GLFW_LIBRARY})
target_link_libraries(HippityHoppity ${SOIL_LIBRARY})

# The rules here are specific to Windows Systems
if(WIN32)
    # Avoid ZERO_CHECK target in Visual Studio
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
 
    # This will use the proper libraries in debug mode in Visual Studio
    set_target_properties(HippityHoppity PROPERTIES DEBUG_POSTFIX _d)
endif(WIN32)
