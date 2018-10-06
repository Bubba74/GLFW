
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

/* Perspective Transformations */
uniform mat4 local;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

void main() {
	gl_Position = vec4(aPos, 1);
	gl_Position = perspective * view * model * local * gl_Position;

	TexCoord = texCoord;

	/*
	if (index < 0.5){
		color = vec4(r, 0, 0, 0.5);
	} else if (index < 1.5){
		color = vec4(0, g, 0, 0.5);
	} else if (index < 2.5){
		color = vec4(0, 0, b, 0.5);
	} else {
		color = vec4(0.1, 0.1, 0.1, 0.5);
	}
	*/
}
