#version 330 core

in vec2 texCoord;    // koordinate teksture
in float frameWidth; // sirina okvira
in vec4 frameColor;  // boja okvira

uniform float time;  // promenljiva za vreme

out vec4 outCol;     // izlazna boja

void main()
{
    float borderCondition = step(frameWidth, texCoord.x) * step(frameWidth, 1.0 - texCoord.x) *      // vraca 1.0 ako je unutrasnji deo gde treba da
                           step(frameWidth, texCoord.y) * step(frameWidth, 1.0 - texCoord.y);        // stoji tekstura, u suprotnom ako je okvir vraca 0.0
                           
    float frequency = 1.0;                                              // frekvencija
    float phase = 0.0;                                                  // pocetak oscilacije
    float red = 0.5 + 0.5 * sin(frequency * time + phase);              // crvena
    float green = 0.5 + 0.5 * sin(frequency * time + phase + 2.0);      // zelena
    float blue = 0.5 + 0.5 * sin(frequency * time + phase + 4.0);       // plava
    vec4 oscillatingColor = vec4(red, green, blue, 1.0);                // sjedinjavanje boja 

    outCol = mix(frameColor + oscillatingColor, vec4(0.0,0.0,0.0,0.0), borderCondition);             // ako je 1.0 postavlja na providnu crnu, inace boji zadatom bojom
}