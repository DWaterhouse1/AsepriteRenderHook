#pragma once

#include <vulkan/vulkan.hpp>

#include "Device.h"
#include "Renderer.h"
#include "Model.h"
#include "Descriptors.h"
#include "UserInterface.h"
#include "Texture.h"
#include "Scene/Scene.h"
#include "Scene/Components.h"

//std
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <mutex>


namespace wrengine
{
/**
* Struct for engine initialisation info.
*/
struct EngineConfigInfo
{
	uint32_t width = 800;
	uint32_t height = 600;
	std::string windowName = "wrengine";
};

/**
* Root class for the rendering engine. All rendering related objects are
* instantiated by Engine. Constructor sets initial window parameters.
*/
class Engine
{
public:
	Engine(const EngineConfigInfo configInfo);

	~Engine();

	// should not copy
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// interface
	void run();
	void addTextureDependency(std::string handle, std::string filePath);
	void addTextureDependency(std::map<std::string, std::string> filePaths);
	void updateTextureData(std::string textureName, std::vector<uint8_t> data);
	std::shared_ptr<ElementManager> getUIManager();
	void loadTextures();
	void loadTexture(
		std::string handle,
		void* data,
		int width,
		int height,
		TextureConfigInfo configInfo);
	std::shared_ptr<Scene> getActiveScene();
	std::shared_ptr<Texture> getTextureByName(const std::string& name);
	void createMaterial(
		const std::string& materialName,
		const std::string& albedoName,
		const std::string& normalMapName);
	Material getMaterialByName(const std::string& name);
	void setNormalCoordinateScales(float x, float y, float z);
	void setPostConstructCallback(std::function<void()> callback);

private:
	void loadEntities();

	// internal functions
	void clearAsyncList();
	void createMaterialDescriptors();

	// window params
	uint32_t m_width = 800;
	uint32_t m_height = 600;
	std::string m_windowName = "wrengine";

	// vulkan/glfw structures
	Window m_window{ m_width, m_height, m_windowName };
	Device m_device{ m_window };
	Renderer m_renderer{ m_window, m_device };
	VkPipelineLayout m_pipelineLayout;

	// note that the pools depend on the device, and must be cleaned up first
	std::unique_ptr<DescriptorPool> m_globalDescriptorPool;
	std::unique_ptr<DescriptorPool> m_textureDescriptorPool;
	std::set<std::pair<std::string, std::string>> m_textureDefinitions;
	std::map<std::string, std::shared_ptr<Texture>> m_textures;
	std::map<std::string, Material> m_materials;
	size_t m_textureCount = 0;
	std::unique_ptr<UserInterface> m_userInterface{};

	// pre frame execution list
	std::vector<std::function<void()>> m_functionList;
	std::mutex m_functionMutex;

	// post construct callback
	std::function<void()> m_postConstructCallback;

	// scene containing the ecs registry
	std::shared_ptr<Scene> m_scene;

	// initialisation flags
	bool m_texturesLoaded;

	// normal map coordinate scales
	glm::vec3 m_coordinateScales = { 1.0f, -1.0f, 1.0f };
	bool m_normalCoordsDirty = false;
};
} // namespace wrengine