#version 330 core

layout(location = 0) in vec2 inPos;

out vec2 progressCoord;

void main()
{
	gl_Position = vec4(inPos, 0.0, 1.0);
	progressCoord = inPos + 0.9;		// standardacija zbog sirine progress bar-a
}