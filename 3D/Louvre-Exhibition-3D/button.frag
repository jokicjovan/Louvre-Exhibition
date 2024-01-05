#version 330 core

uniform vec4 uButtonColor; // promenljiva sa bojom

out vec4 outCol;

void main()
{
	outCol = uButtonColor;
}