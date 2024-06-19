#version 330 core
layout (location = 0) in vec3 vPos;

uniform mat4 uModel;
uniform mat4 uLlightSpaceMatrix;

void main() {
	gl_Position = uLlightSpaceMatrix * uModel * vec4(vPos, 1.0);
}
