
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform float mix_val;
uniform sampler2D crate;
uniform sampler2D face;

void main() {
		// FragColor = color;

		vec4 a = texture(crate, TexCoord);
		vec4 b = texture(face, TexCoord);

		// if (a != b){
		// 	FragColor = vec4(1,0,0,1);
		// } else {
		// 	FragColor = b;
		// }
		FragColor = mix(a, b, mix_val);
}
