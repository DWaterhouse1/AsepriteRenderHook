#version 450
#extension GL_KHR_vulkan_glsl: enable
#extension GL_EXT_debug_printf: enable

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	vec4 color;
} ubo;

layout(push_constant) uniform Push
{
	vec4 lightDir;
	mat2 transform;
	vec2 offset;
	//vec3 color;
} push;

void main()
{
	gl_Position = vec4((push.transform * position) + push.offset, 0.0, 1.0);
	fragColor = color;
	fragTexCoord = uv;
}