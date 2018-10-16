
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 paint;
uniform float mix_val;
uniform sampler2D crate;

void main() {
		// FragColor = color;

		FragColor = texture(crate, TexCoord);

		if (paint.x + paint.y + paint.z > 0.1){
			if (FragColor.r + FragColor.g + FragColor.b > 0.1){
				if (FragColor.a < 0.8)
					FragColor = mix(FragColor, vec4(paint,1), 0.5);
				else
					FragColor = vec4(1,1,1,1);
			}
		}


}
