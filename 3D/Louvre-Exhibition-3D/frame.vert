#version 330 core 

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;
layout(location = 2) in float inWidth;
layout(location = 3) in vec4 inColor;

uniform vec2 circularPosition; // pozicija za rotaciju
uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;


out vec2 texCoord;
out float frameWidth;
out vec4 frameColor;


void main()
{
    vec3 circularPositionWithZ = vec3(circularPosition, 0.0f);
    gl_Position = uP * uV * uM * vec4(inPos + circularPositionWithZ, 1.0f);
    texCoord = inTex;
    frameWidth = inWidth;
    frameColor = inColor;
}