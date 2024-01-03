#version 330 core

uniform vec4 buttonColor; // promenljiva sa bojom

out vec4 outCol;

void main()
{
	outCol = buttonColor;
}