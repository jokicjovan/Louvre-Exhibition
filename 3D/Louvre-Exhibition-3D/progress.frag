#version 330 core

in vec3 progressCoords;        // standardizovane koordinate progressa

out vec4 outColor;

uniform float uProgressValue;  // Vrednost izmedju 0.0 i 1.0

void main()
{                                                                      // float step(float edge, float x);
    float progressColor = step(progressCoords.x, 2*uProgressValue);    // vraca 1.0 ako je trenutna koordinata manja ili jednaka progress value, a inace vraca 0.0
                                                                       // 2*uProgressValue zbog sirine progressBar-a
    outColor = vec4(progressColor, 1.0 - progressColor, 0.0, 1.0);     // ako je trenutna koordinata manja ili jednaka progress value boji crveno, inace zeleno
}
