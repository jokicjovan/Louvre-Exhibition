#version 330 core

layout(location = 0) in vec3 inPos;

out vec3 progressCoords;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    gl_Position = uP * uV * uM * vec4(inPos, 1.0f);
	progressCoords = inPos + 1.9;		// standardacija zbog sirine progress bar-a
}