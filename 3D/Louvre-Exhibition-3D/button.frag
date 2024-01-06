#version 330 core

out vec4 outCol;

uniform vec4 uButtonColor; // promenljiva sa bojom

void main()
{
	outCol = uButtonColor;
}