
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 paint;
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

		if (paint.x + paint.y + paint.z > 0.1){
			if (FragColor.r + FragColor.g + FragColor.b > 0.1 && FragColor.a > 0.8){
				FragColor = mix(FragColor, vec4(paint,1), 0.5);
			}
		}


}
