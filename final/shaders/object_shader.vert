#version 330 core
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexCoords;
layout(location = 2) in vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpaceMatrix;

out vec3 fPos;
out vec2 fTexCoords;
out vec3 fNormal;
out vec4 fPosLightSpace;

void main() {
	vec4 pos = uModel * vec4(vPos, 1.0);
	gl_Position = uProjection * uView * pos;
	fPos = vec3(pos);
	fNormal = mat3(transpose(inverse(uModel))) * vNormal;
	fTexCoords = vTexCoords;
	fPosLightSpace = uLightSpaceMatrix * pos;
}
