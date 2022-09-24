#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

//std
#include <string>

namespace wrengine
{

/**
* Struct containing info relevant to material rendering.
*/
struct Material
{
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normalMap;
	VkDescriptorSet materialDescriptor;
};

/**
* Component representing a 2D position and rotation.
*/
struct Transform2DComponent
{
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 scale{ 1.0f, 1.0f };
	float rotation = 0.0f;
	Transform2DComponent() = default;
	Transform2DComponent(glm::vec2 position, float rotation) :
		position{ position }, rotation{ rotation } {}
};

/**
* Component holding a string tag.
*/
struct TagComponent
{
	std::string tag;
	TagComponent() = default;
	TagComponent(const std::string& tag) :
		tag{ tag } {}
};

/**
* Component for rendering a sprite.
*/
struct SpriteRenderComponent
{
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Texture> normals;
	Material material;

	SpriteRenderComponent() = default;
	SpriteRenderComponent(const glm::vec4& color) : color{ color } {}
};
} // namespace wrengine