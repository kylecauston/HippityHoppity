#version 400

// Attributes passed from the geometry shader
in vec4 frag_color;
in vec2 tex_coord;

// Uniform (global) buffer
uniform sampler2D tex_samp;

void main (void)
{
    // Very simple fragment shader, but we can do anything we want here
    // We could apply a texture to the particle, illumination, etc.

   // gl_FragColor = frag_color;

	// Get pixel from texture
    vec4 outval = texture(tex_samp, tex_coord);
    // Adjust specified object color according to the grayscale texture value
    outval = vec4(outval.r*frag_color.r, outval.g*frag_color.g, outval.b*frag_color.b, sqrt(sqrt(outval.r))*frag_color.a);
    // Set output fragment color
    gl_FragColor = outval;
	//gl_FragColor = vec4(0.9, 0.1, 0.1, 1.0);
}
