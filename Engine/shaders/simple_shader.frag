#version 450

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

void main()
{
	float gamma = 2.2;
	vec3 correctedColor = pow(push.color, vec3(1.0/gamma));
	outColor = vec4(correctedColor, 1.0);
}