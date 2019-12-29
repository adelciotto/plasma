#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragmentPosition;
out vec3 TransformedNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
	TransformedNormal = mat3(transpose(inverse(uModel))) * aNormal;

	Normal = aNormal;
	FragmentPosition = vec3(uModel * vec4(aPosition, 1.0));
}
