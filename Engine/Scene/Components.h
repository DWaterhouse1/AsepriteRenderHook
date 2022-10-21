#pragma once

#include "Texture.h"
#include "Camera.h"

#include <glm/glm.hpp>

//std
#include <string>
#include <functional>
#include <memory>

namespace wrengine
{

/**
* Struct containing info relevant to material rendering.
*/
struct Material
{
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normalMap;
	VkDescriptorSet materialDescriptor = nullptr;
};

/**
* Component representing a 2D position and rotation.
*/
struct TransformComponent
{
	glm::vec3 translation{ 0.0f };
	glm::vec3 scale{ 1.0f };
	glm::vec3 rotation{ 0.0f };
	TransformComponent() = default;
	TransformComponent(glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation) :
		translation{ translation },
		scale{ scale },
		rotation{ rotation }
	{}
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
	glm::vec4 color{ 1.0f };
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Texture> normals;
	Material material;

	SpriteRenderComponent() = default;
	SpriteRenderComponent(const glm::vec4& color) : color{ color } {}
};

// forward declaration
class ScriptableEntity;

/**
* Component containing custom script instance
*/
struct ScriptComponent
{
	std::unique_ptr<ScriptableEntity> instance = nullptr;

	std::function<void()> instantiate;
	std::function<void()> destroy;

	template<typename T>
	void bind()
	{
		instantiate = [&]() { instance = std::make_unique<T>(); };
		destroy	=	[&]() { instance.reset(); };
	}
};

struct PointLightComponent
{
	glm::vec4 lightColor{ 1.0f };
};

enum class ProjectionType
{
	Orthographic,
	Perspective,
};

struct CameraComponent
{
	CameraComponent() = default;
	CameraComponent(const glm::mat4& projection) :
		camera{ std::make_shared<Camera>(projection) }
	{}

	bool isActiveCamera = false;
	ProjectionType projectionType = ProjectionType::Orthographic;
	std::shared_ptr<Camera> camera{};
};
} // namespace wrengine