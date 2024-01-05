#version 330 core 

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;
layout(location = 2) in float inWidth;

uniform vec2 uCircularPosition; // pozicija za rotaciju
uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

out vec2 texCoords;
out float frameWidth;

void main()
{
    vec3 circularPositionWithZ = vec3(uCircularPosition, 0.0f);
    gl_Position =  uP * uV * uM * vec4(inPos + circularPositionWithZ, 1.0f);
    texCoords = inTex;
    frameWidth = inWidth;
}