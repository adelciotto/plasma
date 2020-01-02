#version 330 core

in vec3 Normal;
in vec3 TransformedNormal;
in vec3 FragmentPosition;

uniform float uTime;
uniform float uScale;
uniform vec3 uViewPosition;

out vec4 fragColor;

const float PI = 3.1415926535897932384626433832795;

vec3 plasma(vec2 coords) {
	float val = sin(coords.y + uTime);
	val += sin((coords.x + uTime) * 0.5);
	val += sin((coords.x + coords.y + uTime) * 0.5);
	coords += uScale * 0.5 * vec2(sin(uTime * 0.33), cos(uTime * 0.33));
	val += sin(sqrt(coords.x * coords.x + coords.y * coords.y + 1.0) + uTime);
	val *= 0.5;

	float r, g, b;
	vec3 absNormal = abs(Normal);
	if (absNormal.x == 1.0) {
		r = sin(val * PI);
		g = 1.0;
		b = sin(val * PI);
	} else if (absNormal.y == 1.0) {
		r = 1.0;
		g = sin(val * PI);
		b = sin(val * PI);
	} else if (absNormal.z == 1.0) {
		r = sin(val * PI);
		g = sin(val * PI);
		b = 1.0;
	} else {
		r = g = b = sin(val * 5.0 * PI);
	}

	return vec3(r, g, b) * 0.5 + 0.5;
}

void main() {
	vec2 coords = FragmentPosition.xy;
	coords *= uScale - uScale*0.5;

	vec3 objectColor = plasma(coords);

	const vec3 lightPosition = vec3(1.2, 1.0, 2.0);
	const vec3 lightColor = vec3(1.0);
	const float ambientStrength = 0.05;
	const float specularStrength = 0.5;

	vec3 ambientColor = ambientStrength * lightColor;

	vec3 norm = normalize(TransformedNormal);
	vec3 lightDirection = normalize(lightPosition - FragmentPosition);
	float diff = max(dot(norm, lightDirection), 0.0);
	vec3 diffuseColor = diff * lightColor;

	vec3 viewDirection = normalize(uViewPosition - FragmentPosition);
	vec3 reflectDirection = reflect(-lightDirection, norm);
	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
	vec3 specularColor = specularStrength * spec * lightColor;

	vec3 result = (ambientColor + diffuseColor + specularColor) * objectColor;
	fragColor = vec4(result, 1.0);
}
