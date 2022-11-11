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
	uint config;
	vec4 normalTransform;
} push;

vec4 emmisiveColor(vec4 tex)
{
	return tex;
}

vec4 diffuseColor(vec4 tex)
{
	vec4 normalValue = texture(normalSampler, fragTexCoord);
	normalValue *= push.normalTransform;
	vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;

	for (int i = 0; i < ubo.numLights; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPos;
		float attenuation = 1.0 / dot(directionToLight, directionToLight);
		float cosAoI = max(dot(normalValue.xyz, normalize(directionToLight)), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAoI;
	}

	vec3 preadjustColor = vec3(diffuseLight * tex.rgb);
	float gamma = 2.2;
	vec3 correctedColor = pow(preadjustColor, vec3(1.0/gamma));
	return vec4(correctedColor, tex.a);
}

void main()
{
	vec4 texColor = texture(texSampler, fragTexCoord);

	// using alpha from texcolor as a binary mask on opacity, not using any kind
	// of smooth transparency
	if (texColor.a < 0.01)
	{
		discard;
	}

	switch (push.config)
	{
		case 0:
			outColor = emmisiveColor(texColor); 
			break;
		case 1:
			outColor = diffuseColor(texColor); 
			break;
	}
}