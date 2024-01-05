#version 330 core

in vec2 texCoords;    // koordinate teksture
in float frameWidth; // sirina okvira
in vec4 frameColor;  // boja okvira

uniform float uTime;  // promenljiva za vreme

out vec4 outCol;     // izlazna boja

void main()
{
    float borderCondition = step(frameWidth, texCoords.x) * step(frameWidth, 1.0 - texCoords.x) *      // vraca 1.0 ako je unutrasnji deo gde treba da
                           step(frameWidth, texCoords.y) * step(frameWidth, 1.0 - texCoords.y);        // stoji tekstura, u suprotnom ako je okvir vraca 0.0
                           
    float frequency = 1.0;                                              // frekvencija
    float phase = 0.0;                                                  // pocetak oscilacije
    float red = 0.5 + 0.5 * sin(frequency * uTime + phase);              // crvena
    float green = 0.5 + 0.5 * sin(frequency * uTime + phase + 2.0);      // zelena
    float blue = 0.5 + 0.5 * sin(frequency * uTime + phase + 4.0);       // plava
    vec4 oscillatingColor = vec4(red, green, blue, 1.0);                // sjedinjavanje boja 

    outCol = mix(frameColor + oscillatingColor, vec4(0.0,0.0,0.0,0.0), borderCondition);             // ako je 1.0 postavlja na providnu crnu, inace boji zadatom bojom
}