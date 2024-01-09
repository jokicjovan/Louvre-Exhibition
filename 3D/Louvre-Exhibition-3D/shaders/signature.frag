#version 330 core

in vec2 chTexCoords;

out vec4 outCol;

uniform sampler2D uTex;

void main()
{
	vec4 color = texture(uTex, chTexCoords);
	if (color.a < 0.1){
		discard;
	}
	outCol = vec4(vec3(color), 0.5);
}