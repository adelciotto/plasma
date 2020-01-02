#version 330 core

uniform float uTime;
uniform float uScale;
uniform ivec2 uResolution;

out vec4 fragColor;

const float PI = 3.1415926535897932384626433832795;

void main() {
	vec2 coords = 2.0 * vec2(gl_FragCoord.xy - 0.5 * uResolution.xy) / uResolution.y;
	coords *= uScale - uScale*0.5;

	float val = sin(coords.y + uTime);
	val += sin((coords.x + uTime) * 0.5);
	val += sin((coords.x + coords.y + uTime) * 0.5);
	coords += uScale * 0.5 * vec2(sin(uTime * 0.33), cos(uTime * 0.33));
	val += sin(sqrt(coords.x * coords.x + coords.y * coords.y + 1.0) + uTime);
	val *= 0.5;

	vec3 finalColor = vec3(
		sin(val * PI),
		sin(val * PI + 2.0 * PI * 0.33),
		sin(val * PI + 4.0 * PI * 0.33)
	);

	fragColor = vec4(finalColor*0.5 + 0.5, 1.0);
}
