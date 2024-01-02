#version 330 core 

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;
layout(location = 2) in float inWidth;

uniform vec2 circularPosition; // pozicija za rotaciju

out vec2 texCoord;
out float frameWidth;

void main()
{
    gl_Position = vec4(inPos + circularPosition, 0.0, 1.0);
    texCoord = inTex;
    frameWidth = inWidth;
}