#version 130

// Attributes passed from the vertex shader
in vec4 color_interp;
in vec3 position_interp;
in vec3 normal_interp;
in vec3 light_pos;

uniform float light;

vec4 ambient_color = vec4(0.0, 0.1, 0.1, 1.0);
vec4 diffuse_color = vec4(0.2, 0.2, 0.4, 1.0);
vec4 specular_color = vec4(0.8, 0.8, 0.8, 1.0);
float phong_exponent = 128.0;

void main() 
{
	vec3 N, // Interpolated normal for fragment
         L, // Light-source direction
         V, // View direction
		 R; //reflection vector for lightsource 1
	
	N = normalize(normal_interp);

	L = (light_pos - position_interp); //light direction 1
	L = normalize(L);
	
	R = -L + 2 * dot(L, N) * N; //calculate reflection vector for lightsource 1
	R = normalize(R);

	float Id = max(dot(N, L), 0.0); //calculate diffuse light ratio from both sources

	// Compute specular term for Phong shading
    V = - position_interp; // Eye position is (0, 0, 0) in view coordinates
    V = normalize(V);

	float spec_angle_cos = max(dot(V, R), 0.0); //changed to phong shading (VR instead of NH)
    float Is = pow(spec_angle_cos, phong_exponent);

	// Assign light to the fragment
	float light2 = light;
	if (light2 == 0.9){
		light2 = 0.15;
	} else {
		light2 = 0.0;
		ambient_color = vec4(0.0, 0.0, 0.1, 1.0);
		diffuse_color = vec4(0.0, 0.0, 0.5, 1.0);
		specular_color = vec4(0.8, 0.5, 0.9, 1.0);
	}
    gl_FragColor = light2 * color_interp + ambient_color + Id*diffuse_color + Is*specular_color;
}
