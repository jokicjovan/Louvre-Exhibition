#version 330 core

in vec2 progressCoord;
out vec4 outColor;

uniform float progressValue;  // Vrednost izmedju 0.0 i 1.0

void main()
{                                                                      // float step(float edge, float x);
    float progressColor = step(progressCoord.x, progressValue);        // vraca 1.0 ako je trenutna koordinata manja ili jednaka progress value, a inace vraca 0.0
    outColor = vec4(progressColor, 1.0 - progressColor, 0.0, 1.0);     // ako je trenutna koordinata manja ili jednaka progress value boji crveno, inace zeleno
}
