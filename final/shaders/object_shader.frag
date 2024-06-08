#version 330 core
in vec3 fNormal;
in vec3 fPos;
in vec2 fTexCoords;

#define MAX_POINT_LIGHTS 100

struct Material {
	vec4 color;
	bool useDiffuse;
	sampler2D diffuse;
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
uniform uint lightMask;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform Material material;
uniform vec3 viewPos;

out vec4 FragColor;

vec4 CalcLightInternal(BaseLight light, vec3 lightDirection, vec3 normal) {
	vec4 ambientColor = vec4(material.color.rgb * light.color.rgb * light.ambientIntensity, material.color.a);

	float diffuseFactor = dot(normal, -lightDirection);

	vec4 diffuseColor = vec4(0);
	vec4 specularColor = vec4(0);

	vec4 diffuseTexture;
	if (material.useDiffuse) {
		diffuseTexture = texture(material.diffuse, fTexCoords);
		ambientColor *= diffuseTexture;
	}

	if (diffuseFactor > 0) {
		diffuseColor = vec4(material.color.rgb * light.color.rgb, 1) * light.diffuseIntensity * diffuseFactor;
		if (material.useDiffuse)
		diffuseColor *= diffuseTexture;

		vec3 pixelToCamera = normalize(viewPos - fPos);
		vec3 lightReflect = normalize(reflect(lightDirection, normal));
		float apecularFactor = dot(pixelToCamera, lightReflect);
		if (apecularFactor > 0) {
			apecularFactor = pow(apecularFactor, material.shininess);

			specularColor = material.color * light.color * light.diffuseIntensity * apecularFactor;
			if (material.useDiffuse)
			specularColor.a *= diffuseTexture.a;
			if (material.useSpecular)
			specularColor *= texture(material.specular, fTexCoords);
		}
	}
	return (ambientColor + diffuseColor + specularColor);
}

vec4 CalcDirectionalLight(vec3 normal) {
	return CalcLightInternal(dirLight.base, dirLight.direction, normal);
}

vec4 CalcPointLight(int index, vec3 normal) {
	vec3 lightDirection = fPos - pointLights[index].localPos;
	float distance = length(lightDirection);
	lightDirection = normalize(lightDirection);
	float attenuation = 1 / (pointLights[index].atten.constant +
	pointLights[index].atten.linear * distance +
	pointLights[index].atten.exp * distance * distance);

	vec4 color = CalcLightInternal(pointLights[index].base, lightDirection, normal);
	color *= attenuation;
	
	return color;
}

void main() {
	vec3 normal = normalize(fNormal);
	vec4 totalLight = (lightMask & 0x1u) > 0u ? CalcDirectionalLight(normal) : vec4(0);
	
	if ((lightMask & 0x2u) > 0u)
	for (int i = 0; i < pointLightsLength; i++) {
		totalLight += CalcPointLight(i, normal);
	}

	FragColor = totalLight;
}