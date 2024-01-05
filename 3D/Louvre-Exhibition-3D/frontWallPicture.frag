#version 330 core

in vec2 texCoords;       // koordinate teksture
in float frameWidth;    // sirina okvira

uniform sampler2D uTex; // promenljiva teksturna jedinica

out vec4 outCol;        // izlazna boja

void main()
{
    float borderCondition = step(frameWidth, texCoords.x) * step(frameWidth, 1.0 - texCoords.x) *       // vraca 1.0 ako je unutrasnji deo gde treba da 
                           step(frameWidth, texCoords.y) * step(frameWidth, 1.0 - texCoords.y);         // stoji tekstura, u suprotnom ako je okvir vraca 0.0
    vec4 picture = texture(uTex, texCoords);                                                   
    outCol = mix(vec4(0.0,0.0,0.0,0.0), picture, borderCondition);                                    // ako je 1.0 boji sa teksturom, inace postavlja na providnu crnu
}