#version 330 core

in vec2 chTexCoords;		// koordinate teksture

out vec4 outCol;

uniform sampler2D uTex; // teksturna jedinica

void main()
{
	outCol = vec4(vec3(texture(uTex, chTexCoords)), 0.5);
}