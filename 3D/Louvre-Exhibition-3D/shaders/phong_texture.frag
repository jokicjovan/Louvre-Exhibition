#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emissive;
    float shininess;
}; 

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

in vec3 chNor;
in vec3 chFragPos;
in vec2 chTexCoords;

out vec4 outCol;

uniform SpotLight uCameraSpotLight;
uniform SpotLight uCeilingSpotLight;
uniform PointLight uFloorPointLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec4 texColor = texture(uMaterial.diffuse, chTexCoords);
    if(texColor.a < 0.1)
        discard;

    vec3 normal = normalize(chNor);
    vec3 viewDir = normalize(uViewPos - chFragPos);

    vec3 result = CalcSpotLight(uCeilingSpotLight, normal, chFragPos, viewDir);
    result += CalcSpotLight(uCameraSpotLight, normal, chFragPos, viewDir);
    result += CalcPointLight(uFloorPointLight, normal, chFragPos, viewDir);
    //result += vec3(texture(uMaterial.emissive, chTexCoords));

    outCol = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(uMaterial.diffuse, chTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(uMaterial.diffuse, chTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, chTexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient = light.ambient * vec3(texture(uMaterial.diffuse, chTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(uMaterial.diffuse, chTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, chTexCoords));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}