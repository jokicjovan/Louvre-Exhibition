#version 330 core

struct Light{ //Svjetlosni izvor
	vec3 position; //Pozicija
    vec3 direction; //Direkcija
    float cutOff;

	vec3 ambient; //Ambijentalna komponenta (Indirektno svjetlo)
	vec3 diffuse; //Difuzna komponenta (Direktno svjetlo)
	vec3 specular; //Spekularna komponenta (Odsjaj)

	float constant;
    float linear;
    float quadratic;
};

struct Material{ //Materijal objekta
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shine; //Uglancanost
};

in vec3 chNor;
in vec3 chFragPos;

out vec4 outCol;

uniform Light uLight;
uniform Material uMaterial;
uniform vec3 uViewPos;	//Pozicija kamere (za racun spekularne komponente)

void main()
{
	vec3 lightDirection = normalize(uLight.position - chFragPos);
	float theta = dot(lightDirection, normalize(-uLight.direction));

	float distance  = length(uLight.position - chFragPos);
	float attenuation = 1.0 / (uLight.constant + uLight.linear * distance + 
    		    uLight.quadratic * (distance * distance));    


	vec3 resA = uLight.ambient * uMaterial.ambient;
	resA *= attenuation; 

	if(theta > uLight.cutOff) 
	{     
		vec3 normal = normalize(chNor);
		float nD = max(dot(normal, lightDirection), 0.0);
		vec3 resD = uLight.diffuse * ( nD * uMaterial.diffuse);		
		resD *= attenuation;

		vec3 viewDirection = normalize(uViewPos - chFragPos);
		vec3 reflectionDirection = reflect(-lightDirection, normal);
		float s = pow(max(dot(viewDirection, reflectionDirection), 0.0), uMaterial.shine);
		vec3 resS = uLight.specular * (s * uMaterial.specular);
		resS *= attenuation;

		outCol = vec4(resA + resD + resS, 1.0); //Fongov model sjencenja
	}
	else 
	{
		outCol = vec4(resA, 1.0);
	}

}