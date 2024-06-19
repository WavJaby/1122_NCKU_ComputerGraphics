#version 330 core
in vec3 fPos;
in vec2 fTexCoords;
in vec3 fNormal;
in vec4 fPosLightSpace;

#define MAX_POINT_LIGHTS 100

struct Material {
	vec4 color;
	bool useDiffuse;
	sampler2D diffuse;
	bool singleChannel;
	bool useSpecular;
	sampler2D specular;
	float shininess;
};

struct BaseLight {
	vec4 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirLight {
	BaseLight base;
	vec3 direction;
};

struct Attenuation {
	float constant;
	float linear;
	float exp;
};

struct PointLight {
	BaseLight base;
	vec3 localPos;
	Attenuation atten;
};

uniform DirLight dirLight;
uniform int pointLightsLength;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform Material material;
uniform sampler2D uShadowMap;
uniform vec3 uViewPos;

out vec4 FragColor;

float ShadowCalculation(vec4 fPosLightSpace, float bias) {
    // perform perspective divide
	vec3 projCoords = fPosLightSpace.xyz / fPosLightSpace.w;
    // transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(uShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

vec4 CalcLightInternal(BaseLight light, vec3 lightDirection, vec3 normal) {
	vec4 ambientColor = vec4(material.color.rgb * light.color.rgb * light.ambientIntensity, material.color.a);

	float diffuseFactor = dot(normal, -lightDirection);

	vec4 diffuseColor = vec4(0);
	vec4 specularColor = vec4(0);

	vec4 diffuseTexture;
	if(material.useDiffuse) {
		diffuseTexture = texture(material.diffuse, fTexCoords);
		if(material.singleChannel)
			diffuseTexture = vec4(diffuseTexture.r, diffuseTexture.r, diffuseTexture.r, 1);
		if(diffuseTexture.a == 0)
			discard;
	}

	if(diffuseFactor > 0) {
		diffuseColor = vec4(material.color.rgb * light.color.rgb, 1) * light.diffuseIntensity * diffuseFactor;

		vec3 pixelToCamera = normalize(uViewPos - fPos);
		vec3 lightReflect = normalize(reflect(lightDirection, normal));
		float apecularFactor = dot(pixelToCamera, lightReflect);
		if(apecularFactor > 0 && material.shininess >= 1) {
			apecularFactor = pow(apecularFactor, material.shininess);

			specularColor = material.color * light.color * light.diffuseIntensity * apecularFactor;
			if(material.useSpecular)
				specularColor *= texture(material.specular, fTexCoords);
		}
	}

	// float bias = max(0.005 * (1.0 - dot(normal, lightDirection)), 0.0005);  
	float shadow = ShadowCalculation(fPosLightSpace, 0.00004);
	return (ambientColor + (1.0 - shadow) * (diffuseColor + specularColor)) * diffuseTexture;
}

vec4 CalcDirectionalLight(vec3 normal) {
	return CalcLightInternal(dirLight.base, dirLight.direction, normal);
}

vec4 CalcPointLight(int index, vec3 normal) {
	vec3 lightDirection = fPos - pointLights[index].localPos;
	float dist = length(lightDirection);
	lightDirection = normalize(lightDirection);
	float attenuation = 1 / (pointLights[index].atten.constant +
		pointLights[index].atten.linear * dist +
		pointLights[index].atten.exp * dist * dist);

	vec4 color = CalcLightInternal(pointLights[index].base, lightDirection, normal);
	color *= attenuation;

	return color;
}

void main() {
	vec3 normal = normalize(fNormal);
	vec4 totalLight = CalcDirectionalLight(normal);

	for(int i = 0; i < pointLightsLength; i++) {
		totalLight += CalcPointLight(i, normal);
	}

	FragColor = totalLight;
}