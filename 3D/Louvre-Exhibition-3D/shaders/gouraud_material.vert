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

struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor;

out vec3 chCol;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;
uniform SpotLight uCameraSpotLight;
uniform SpotLight uCeilingSpotLight;
uniform PointLight uFloorPointLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 fragPos = vec3(uM * vec4(inPos, 1.0));
	vec3 nor = mat3(transpose(inverse(uM))) * inNor;
	gl_Position = uP * uV * vec4(fragPos, 1.0);

    vec3 normal = normalize(nor);
    vec3 viewDir = normalize(uViewPos - fragPos);

    vec3 result = CalcSpotLight(uCeilingSpotLight, normal, fragPos, viewDir);
    result += CalcSpotLight(uCameraSpotLight, normal, fragPos, viewDir);
    result += CalcPointLight(uFloorPointLight, normal, fragPos, viewDir);

    chCol = result;
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
    vec3 ambient = light.ambient * uMaterial.ambient;
    vec3 diffuse = light.diffuse * diff * uMaterial.diffuse;
    vec3 specular = light.specular * spec * uMaterial.specular;
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
    vec3 ambient = light.ambient * uMaterial.ambient;
    vec3 diffuse = light.diffuse * diff * uMaterial.diffuse;
    vec3 specular = light.specular * spec * uMaterial.specular;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}  