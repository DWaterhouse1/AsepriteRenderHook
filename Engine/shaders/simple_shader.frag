#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;

layout(push_constant) uniform Push
{
	vec4 lightDir;
	//mat2 transform;
	//vec2 offset;
	//vec3 color;
} push;

void main()
{
	vec4 texColor = texture(texSampler, fragTexCoord);
	vec4 normalValue = texture(normalSampler, fragTexCoord);

	if (texColor.a < 0.1)
	{
		discard;
	}

	float adjustment = (1 + dot(push.lightDir.xyz, normalValue.xyz)) / 2;

	float gamma = 2.2;
	vec3 correctedColor = pow(adjustment * texColor.rgb, vec3(1.0/gamma));
	outColor = vec4(correctedColor, texColor.a);
}