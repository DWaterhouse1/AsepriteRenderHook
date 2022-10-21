#include "Engine.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// systems
#include "RenderSystem.h"
#include "PointLightSystem.h"

#include <imgui.h>

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <tuple>
#include <chrono>

namespace wrengine
{
Engine::Engine(const EngineConfigInfo configInfo) :
	m_width{ configInfo.width },
	m_height{ configInfo.height },
	m_windowName{ configInfo.windowName }
{
	m_globalDescriptorPool =
		DescriptorPool::Builder(m_device)
		.setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT * 2)
		.addPoolSize(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			Swapchain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			Swapchain::MAX_FRAMES_IN_FLIGHT)
		.build();

	m_userInterface = std::make_unique<UserInterface>(
		m_window,
		m_device,
		m_renderer.getSwapchainRenderPass(),
		static_cast<uint32_t>(m_renderer.getImageCount()));
}

Engine::~Engine()
{
}

// Interface start ----------------------------------

/**
* Starts up all major engine functions. Requires the engine configurations
* such as resource dependencies to be completed before running. This function
* contains the main loop, and so will block until exit is requested by the
* glfw window.
*/
void Engine::run()
{
	if (!m_texturesLoaded) loadTextures();

	std::vector<std::unique_ptr<Buffer>> uboBuffers(
		Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); ++i)
	{
		uboBuffers[i] = std::make_unique<Buffer>(
			m_device,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
		.addBinding(
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();
	
	std::vector<VkDescriptorSet> globalDescriptorSets(
		Swapchain::MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < globalDescriptorSets.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();

		DescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	auto materialSetLayout = DescriptorSetLayout::Builder(m_device)
		.addBinding(
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	auto materialView = m_scene->getAllEntitiesWith<SpriteRenderComponent>();
	for (auto&& [entity, renderComponent] : materialView.each())
	{
		Material& material = renderComponent.material;
		VkDescriptorImageInfo albedoInfo = material.albedo->descriptorInfo();
		VkDescriptorImageInfo normalsInfo = material.normalMap->descriptorInfo();

		DescriptorWriter(*materialSetLayout, *m_textureDescriptorPool)
			.writeImage(0, &albedoInfo)
			.writeImage(1, &normalsInfo)
			.build(material.materialDescriptor);
	}

	RenderSystem renderSystem{ 
		m_device,
		m_renderer.getSwapchainRenderPass(),
		globalSetLayout->getDescriptorSetLayout(),
		materialSetLayout->getDescriptorSetLayout(),
		m_scene
	};

	PointLightSystem pointLightSystem{ m_scene };

	m_scene->onSceneStart();

	std::chrono::steady_clock::time_point currentTime =
		std::chrono::high_resolution_clock::now();

	while (!m_window.shouldClose())
	{
		// note glfwPollEvents() may block, e.g. on window resize
		glfwPollEvents();

		// -------------- frame timing --------------
		std::chrono::steady_clock::time_point newTime =
			std::chrono::high_resolution_clock::now();
		
		float frameTime = 
			std::chrono::duration<
			float,
			std::chrono::seconds::period>(
				newTime - currentTime).count();
		currentTime = newTime;

		clearAsyncList();

		m_userInterface->getElementManager()->tickElements(frameTime);

		// -------------  begin frame --------------
		if (VkCommandBuffer commandBuffer = m_renderer.beginFrame())
		{
			int frameIndex = m_renderer.getFrameIndex();
			FrameInfo frameInfo
			{
				frameIndex,
				frameTime,
				commandBuffer,
				globalDescriptorSets[frameIndex]
			};

			// FRAME PHASE 1
			// prepare and update objects in memory
			m_scene->onUpdate(frameTime);
			float aspect = m_renderer.getAspectRatio();
			m_scene->getActiveCamera()->setOrthographicProjection(
				-aspect, aspect,
				-1, 1,
				-1, 1);

			GlobalUbo ubo{};
			pointLightSystem.update(ubo);
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			// FRAME PHASE 2
			// render the frame
			m_userInterface->startFrame();
			m_renderer.beginSwapchainRenderPass(commandBuffer);
			renderSystem.renderEntities(frameInfo);
			m_userInterface->getElementManager()->runElements();
			m_userInterface->render(commandBuffer);
			m_renderer.endSwapchainRenderPass(commandBuffer);
			m_renderer.endFrame();
			m_userInterface->endFrame();
		}
	}
	vkDeviceWaitIdle(m_device.device());
}

/**
* Updates the associated texture data held by the texture with the supplied
* name. Can be called asyncrhonously. Note that this uses C++14 generalized
* lambda captures.
* 
* @param textureName The name of the texture to update.
* @param data Vector containing the data
*/
void Engine::updateTextureData(
	std::string textureName,
	std::vector<uint8_t> data)
{
	std::function<void()> f_update =
		[texData = std::move(data), texture = m_textures[textureName]]
			{
				texture->updateTextureData((void*)texData.data());
			};

	std::scoped_lock<std::mutex> lock(m_functionMutex);
	m_functionList.push_back(f_update);
}

/**
* Adds a single texture dependency to the engine. The texture will be loaded
* on engine startup.
* 
* @param handle The name of the texture, used as a key in the texture map.
* @param filePath The file path from which to attempt to load textures.
*/
void Engine::addTextureDependency(std::string handle, std::string filePath)
{
	m_textureDefinitions.insert(std::make_pair(handle, filePath));
}

/**
* Adds a set of texture dependencies to the engine. The textures will be loaded
* on engine startup.
*
* @param filePaths A map of name/filepath key/value pairs to be added to the
* engine texture dependencies.
*/
void Engine::addTextureDependency(std::map<std::string, std::string> filePaths)
{
	m_textureDefinitions.insert(filePaths.begin(), filePaths.end());
}

std::shared_ptr<ElementManager> Engine::getUIManager()
{
	return m_userInterface->getElementManager();
}

/**
* Gets the currently active scene object. Will lazy initialize a new scene if
* no currently active scene exists.
* 
* @return Shared ptr to the active scene object.
*/
std::shared_ptr<Scene> Engine::getActiveScene()
{
	if (!m_scene)
	{
		m_scene = std::make_shared<Scene>();
	}

	return m_scene;
}

/**
* Finds a Texture object belonging to the supplied name.
* 
* @param name The name of the texture to find.
* 
* @return Shared ptr to the texture object with name.
*/
std::shared_ptr<Texture> Engine::getTextureByName(const std::string& name)
{
	auto texIt = m_textures.find(name);
	assert(texIt != m_textures.end() && "failed to find texture!");
	return m_textures[name];
}

/**
* Creates a material from supplied textures.
* 
* @param materialName The name of the created material.
* @param albedoName The name of the texture to use as material albedo.
* @param normalMapName The name of the texture to use as the materials normal
*	map.
*/
void Engine::createMaterial(
	const std::string& materialName,
	const std::string& albedoName,
	const std::string& normalMapName)
{
	Material newMaterial;
	newMaterial.albedo = getTextureByName(albedoName);
	newMaterial.normalMap = getTextureByName(normalMapName);
	m_materials.insert({ materialName, newMaterial });
}

/**
* Finds a Material object belonging to the supplied name.
*
* @param name The name of the material to find.
*
* @return The material object with name.
*/
Material Engine::getMaterialByName(const std::string& name)
{
	auto texIt = m_materials.find(name);
	assert(texIt != m_materials.end() && "failed to find material!");
	return m_materials[name];
}

//  Interface end  ----------------------------------

/**
* Itterates over registered texture dependencies and loads the files into
* Texture objects. These objects are inserted into a map as values with their
* registered names as keys.
*/
void Engine::loadTextures()
{
	m_textureCount = m_textureDefinitions.size();
	for (auto& [handle, filePath] : m_textureDefinitions)
	{
		m_textures.emplace(handle, std::make_shared<Texture>(m_device));
		m_textures[handle]->loadFromFile(filePath);
	}

	m_textureDescriptorPool = DescriptorPool::Builder(m_device)
		.setMaxSets(m_textureCount)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_textureCount)
		.build();

	m_texturesLoaded = true;
}

/**
* Itterates over configured materials and creates the required descriptors for
* each.
*/
void Engine::createMaterialDescriptors()
{
	// TODO figure this out
}

/**
* Creates and configures renderable entities.
*/
void Engine::loadEntities()
{
	/**
	* TODO use this method to load more complicated entity data. Depends on
	* asset manager system so data can be saved and later deserialized here.
	* Currrently only rendering quads for sprites so not necessary yet.
	*/
}

void Engine::clearAsyncList()
{
	std::scoped_lock<std::mutex> lock(m_functionMutex);
	for (auto& function : m_functionList)
	{
		function();
	}

	std::vector<std::function<void()>>().swap(m_functionList);
}
} // namespace wrengine