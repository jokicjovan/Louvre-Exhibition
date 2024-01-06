#version 330 core

struct Light{ //Svjetlosni izvor
	vec3 position;  //Pozicija
    vec3 direction; //Direkcija
    float cutOff;
    float outerCutOff;

	vec3 ambient;   //Ambijentalna komponenta (Indirektno svjetlo)
	vec3 diffuse;   //Difuzna komponenta (Direktno svjetlo)
	vec3 specular;  //Spekularna komponenta (Odsjaj)

	float constant;
    float linear;
    float quadratic;
};

struct Material{ //Materijal objekta
	vec3 diffuse;
	vec3 specular;
	float shininess; //Uglancanost
};

in vec3 chNor;
in vec3 chFragPos;

out vec4 outCol;

uniform Light uLight;
uniform Material uMaterial;
uniform vec3 uViewPos;	//Pozicija kamere (za racun spekularne komponente)

vec3 calculateOutColor() {
	// ambient
    vec3 ambient = uLight.ambient * uMaterial.diffuse;
    
    // diffuse
    vec3 norm = normalize(chNor);
    vec3 lightDir = normalize(uLight.position - chFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uLight.diffuse * diff * uMaterial.diffuse;  

    // specular
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);
    vec3 specular = uLight.specular * spec * uMaterial.specular; 

	// spotlight (soft edges)
    float theta = dot(lightDir, normalize(-uLight.direction)); 
    float epsilon = (uLight.cutOff - uLight.outerCutOff);
    float intensity = clamp((theta - uLight.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

	// attenuation
    float distance = length(uLight.position - chFragPos);
    float attenuation = 1.0 / (uLight.constant + uLight.linear * distance + uLight.quadratic * (distance * distance));    
    ambient *= attenuation; 
    diffuse *= attenuation;
    specular *= attenuation; 

	return ambient + diffuse + specular;
}

void main()
{
    outCol = vec4(calculateOutColor(), 1.0);
}