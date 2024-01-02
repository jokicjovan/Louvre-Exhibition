#version 330 core

in vec2 texCoord;       // koordinate teksture
in float frameWidth;    // sirina okvira

uniform sampler2D uTex; // promenljiva teksturna jedinica

out vec4 outCol;        // izlazna boja

void main()
{
    float borderCondition = step(frameWidth, texCoord.x) * step(frameWidth, 1.0 - texCoord.x) *       // vraca 1.0 ako je unutrasnji deo gde treba da 
                           step(frameWidth, texCoord.y) * step(frameWidth, 1.0 - texCoord.y);         // stoji tekstura, u suprotnom ako je okvir vraca 0.0
    vec4 picture = texture(uTex, texCoord);                                                   
    outCol = mix(vec4(0.0,0.0,0.0,0.0), picture, borderCondition);                                    // ako je 1.0 boji sa teksturom, inace postavlja na providnu crnu
}