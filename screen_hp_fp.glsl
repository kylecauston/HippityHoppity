#version 130

// Passed from the vertex shader
in vec2 uv0;

// Passed from outside
uniform float hp;
uniform sampler2D texture_map;

void main() 
{
    vec2 pos = uv0;

    vec4 pixel = texture(texture_map, pos);
	
	if (pos.y > 0.03333 && pos.y < 0.075 && pos.x > 0.375 && pos.x < (hp / 100.0) * 0.625){
		gl_FragColor = vec4(0.0,1.0,0.0,1.0); //green hp bar
	}
	else if (pos.y > 0.075 && pos.y < 0.08 && pos.x > 0.37 && pos.x < 0.63){
		gl_FragColor = vec4(0.0,0.0,0.0,1.0); //top border
	}
	else if (pos.y > 0.02833 && pos.y < 0.03333 && pos.x > 0.37 && pos.x < 0.63){
		gl_FragColor = vec4(0.0,0.0,0.0,1.0); //bottom border
	}
	else if (pos.y > 0.03333 && pos.y < 0.075 && pos.x > 0.37 && pos.x < 0.375){
		gl_FragColor = vec4(0.0,0.0,0.0,1.0); //left border
	}
	else if (pos.y > 0.03333 && pos.y < 0.075 && pos.x > 0.625 && pos.x < 0.63){
		gl_FragColor = vec4(0.0,0.0,0.0,1.0); //right border
	}
	else {
		gl_FragColor = pixel;
	}
}
