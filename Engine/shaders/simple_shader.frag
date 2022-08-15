#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform Push
{
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

void main()
{
	vec4 texColor = texture(texSampler, fragTexCoord);
	float gamma = 2.2;
	vec3 correctedColor = pow(texColor.rgb, vec3(1.0/gamma));
	outColor = vec4(correctedColor, texColor.a);
}