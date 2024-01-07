#version 330 core 

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTexCoords;  //Koordinate teksture, propustamo ih u FS kao boje

out vec2 chTexCoords;

void main()
{
	gl_Position = vec4(inPos.x, inPos.y, 0.0f, 1.0f);
	chTexCoords = inTexCoords;
}