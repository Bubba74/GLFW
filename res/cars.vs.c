
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 transform;

void main() {
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	gl_Position = transform * gl_Position;

	TexCoord = texCoord;
	TexCoord.y = 1 - TexCoord.y;

}
