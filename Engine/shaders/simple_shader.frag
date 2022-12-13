#version 450
#extension GL_KHR_vulkan_glsl: enable
#extension GL_EXT_debug_printf: enable

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
	mat4 projView;
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
	vec4 normalMap = texture(normalSampler, fragTexCoord);
	//vec4 normalMap = vec4(0.0, 0.0, 1.0, 0.0);
	PointLight light = ubo.pointLights[0];

	vec3 lightDir = vec3(light.position.xyz - fragPos);

	float D = length(lightDir);
	vec3 N = normalize(2 * normalMap.rgb - 1);
	vec3 L = normalize(lightDir);

	N *= push.normalTransform.xyz;

	vec3 diffuse = (light.color.rgb * light.color.a) * max(acos((dot(N, L))), 0);
	//vec3 diffuse = (light.color.rgb * light.color.a) * max(dot(N, L), 0);

	vec3 ambient = ubo.ambientLight.rgb * ubo.ambientLight.a;

	float attenuation = 50000.0 / (0.4 + (3 * D) + (20 * D * D));

	vec3 intensity = ambient + (diffuse * attenuation);
	
	//return vec4(vec3(0.2) * intensity, tex.a);
	return vec4(tex.rgb * intensity, tex.a);
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

	vec4 color;

	switch (push.config)
	{
		case 0:
			color = emmisiveColor(texColor); 
			break;
		case 1:
			color = diffuseColor(texColor); 
			break;
	}

	// gamma correction
	float gamma = 2.2;
	vec3 correctedColor = pow(color.rgb, vec3(1.0 / gamma));
	outColor = vec4(correctedColor, color.a);
}