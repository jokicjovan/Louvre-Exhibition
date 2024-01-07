#version 330 core 

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoords;
layout(location = 2) in float inWidth;
layout(location = 3) in vec4 inColor;

out vec2 chTexCoords;
out float chFrameWidth;
out vec4 chFrameColor;

uniform vec2 uCircularPosition; // pozicija za rotaciju
uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    vec3 circularPositionWithZ = vec3(uCircularPosition, 0.0f);
    gl_Position = uP * uV * uM * vec4(inPos + circularPositionWithZ, 1.0f);
    chTexCoords = inTexCoords;
    chFrameWidth = inWidth;
    chFrameColor = inColor;
}