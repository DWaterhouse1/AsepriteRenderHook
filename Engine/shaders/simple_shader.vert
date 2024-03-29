#version 450
#extension GL_KHR_vulkan_glsl: enable
#extension GL_EXT_debug_printf: enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragTexCoord;

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

layout(push_constant) uniform Push
{
	mat4 model;
	uint config;
	vec4 normalTransform;
} push;

void main()
{
	vec4 vertexWorldPosition = push.model * vec4(position, 1.0);
	gl_Position = ubo.projView * vertexWorldPosition;
	fragPos = vertexWorldPosition.xyz;
	//debugPrintfEXT("fragPos.x : %f", fragPos.x);
	//debugPrintfEXT("fragPos.y : %f", fragPos.y);
	fragTexCoord = uv;
}