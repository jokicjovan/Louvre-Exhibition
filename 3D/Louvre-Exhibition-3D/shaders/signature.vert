#version 330 core 

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoords;

out vec2 chTexCoords;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
	chTexCoords = inTexCoords;
	gl_Position =  uM * uV * uP * vec4(inPos, 1.0);
}