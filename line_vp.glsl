#version 400

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 normal_mat;
uniform float timer;
uniform float red;
uniform float green;
uniform float blue;

// Attributes forwarded to the geometry shader
out vec3 vertex_color;
out float timestep;
out float particle_id;

// Simulation parameters (constants)
uniform vec3 up_vec = vec3(0.0, 1.0, 0.0);
uniform vec3 object_color = vec3(0.8, 0.8, 0.8);
float accel = 1.3;
float speed = 1.2;

const float pi = 3.1415926536;
const float pi_over_two = 1.5707963268;
const float two_pi = 2.0*pi;

void main()
{
	particle_id = color.r;
    vec4 norm = normal_mat * vec4(normal, 1.0);

	// Define particle id
    particle_id = color.r; // Derived from the particle color. We use the id to keep track of particles

    // Define time in a cyclic manner
    float phase = two_pi*particle_id; // Start the sin wave later depending on the particle_id
    float param = timer / 10.0 + phase; // The constant that divides "timer" also helps to adjust the "speed" of the fire
    float rem = mod(param, pi_over_two); // Use the remainder of dividing by pi/2 so that we are always in the range [0..pi/2] where sin() gives values in [0..1]
    float circtime = sin(rem); // Get time value in [0..1], according to a sinusoidal wave
                                    
    // Set up parameters of the particle motion
    float t = abs(circtime)*(0.3 + abs(normal.y)); // Our time parameter

    // First, work in local model coordinates (do not apply any transformation)
    vec3 position = vertex;
    position += speed*up_vec*accel*t*t; // Particle moves up
    
    // Define output position but do not apply the projection matrix yet
    gl_Position = view_mat * world_mat * vec4(position, 1.0);

    // Define color of vertex
	vertex_color = vec3(red, green, blue);

    // Forward time step to geometry shader
    timestep = t;
}