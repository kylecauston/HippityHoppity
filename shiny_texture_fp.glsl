#version 130

// Attributes passed from the vertex shader
in vec4 color_interp;
in vec2 uv_interp;

// Uniform (global) buffer
uniform sampler2D texture_map;

void main() 
{
    // Retrieve texture value
    vec4 pixel = texture(texture_map, uv_interp);

    // Use texture in determining fragment colour
	gl_FragColor = pixel;
}