#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
	vec4 ambientLight;
	PointLight[10] pointLights;
	int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;

layout(push_constant) uniform Push
{
	mat4 transform;
} push;

void main()
{
	vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;

	vec4 texColor = texture(texSampler, fragTexCoord);
	vec4 normalValue = texture(normalSampler, fragTexCoord);

	// TODO actual transparency instead of this silly disarcd thing
	if (texColor.a < 0.1)
	{
		discard;
	}

	for (int i = 0; i < ubo.numLights; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPos;
		float attenuation = 1.0 / dot(directionToLight, directionToLight);
		float cosAoI = max(dot(normalValue.xyz, normalize(directionToLight)), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAoI;
	}

	vec3 preadjustColor = vec3(diffuseLight * texColor.rgb);
	float gamma = 2.2;
	vec3 correctedColor = pow(preadjustColor, vec3(1.0/gamma));
	outColor = vec4(correctedColor, texColor.a);
}