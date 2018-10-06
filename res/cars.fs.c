
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 carColor;
uniform sampler2D carTexture;

void main() {

		FragColor = texture(carTexture, TexCoord);
		// if (FragColor == vec4(1, 1, 1, 1))
			// FragColor = vec4(carColor,1.0);
		if (FragColor.a > 0.1)
			FragColor *= vec4(carColor, 1);
		// vec4 vals = vec4(1,1,1,1) - FragColor;
}
