#version 330 core

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

in vec2 chTexCoords;
in float chFrameWidth;
in vec4 chFrameColor;
in vec3 chNor;
in vec3 chFragPos;

uniform float uTime;
uniform sampler2D uTex;
uniform vec3 uViewPos;
uniform SpotLight uCameraSpotLight;
uniform SpotLight uCeilingSpotLight;
uniform PointLight uFloorPointLight;

out vec4 outCol;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture);

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
    vec4 pictureWithFrame = mix(chFrameColor + oscillatingColor, picture, borderCondition);     // ako je 1.0 postavlja na teksturu, inace boji zadatom bojom

    vec3 normal = normalize(chNor);
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 result = CalcSpotLight(uCeilingSpotLight, normal, chFragPos, viewDir, vec3(pictureWithFrame));
    result += CalcPointLight(uFloorPointLight, normal, chFragPos, viewDir, vec3(pictureWithFrame));
    result += CalcSpotLight(uCameraSpotLight, normal, chFragPos, viewDir, vec3(pictureWithFrame));
    if(result.x == 0.0f && result.y == 0.0f && result.y == 0.0f){
        discard;
    }
    else{
        outCol = vec4(result, 1.0);
    }
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0f);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  
    
    // combine results
    vec3 ambient = light.ambient * texture;
    vec3 diffuse = light.diffuse * diff * texture;
    vec3 specular = light.specular * spec * texture;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0f);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  
    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient = light.ambient * texture;
    vec3 diffuse = light.diffuse * diff * texture;
    vec3 specular = light.specular * spec * texture;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}  