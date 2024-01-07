#version 330 core

in vec2 chTexCoords;    // koordinate teksture
in float chFrameWidth; // sirina okvira
in vec4 chFrameColor;  // boja okvira

uniform float uTime;  // promenljiva za vreme
uniform sampler2D uTex; // promenljiva teksturna jedinica

out vec4 outCol;     // izlazna boja

void main()
{
    float borderCondition = step(chFrameWidth, chTexCoords.x) * step(chFrameWidth, 1.0 - chTexCoords.x) *      // vraca 1.0 ako je unutrasnji deo gde treba da
                           step(chFrameWidth, chTexCoords.y) * step(chFrameWidth, 1.0 - chTexCoords.y);        // stoji tekstura, u suprotnom ako je okvir vraca 0.0
                           
    float frequency = 1.0;                                              // frekvencija
    float phase = 0.0;                                                  // pocetak oscilacije
    float red = 0.5 + 0.5 * sin(frequency * uTime + phase);             // crvena
    float green = 0.5 + 0.5 * sin(frequency * uTime + phase + 2.0);     // zelena
    float blue = 0.5 + 0.5 * sin(frequency * uTime + phase + 4.0);      // plava
    vec4 oscillatingColor = vec4(red, green, blue, 1.0);                // sjedinjavanje boja 

    vec4 picture = texture(uTex, chTexCoords);
    outCol = mix(chFrameColor + oscillatingColor, picture, borderCondition);             // ako je 1.0 postavlja na providnu crnu, inace boji zadatom bojom
}