// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

#include "PointLightController.h"
#include "SpriteController.h"
#include "MainWindow.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

AsepriteRenderHook::AsepriteRenderHook()
{
	wrengine::EngineConfigInfo configInfo{};
	configInfo.height = HEIGHT;
	configInfo.width = WIDTH;
	configInfo.windowName = "Aseprite Render Hook";
	m_engine = std::make_shared<wrengine::Engine>(configInfo);
}

void AsepriteRenderHook::run()
{
	initServer();
	std::unique_lock lock(m_conditionMutex);
	std::cout << "waiting on init msg from aseprite\n";
	m_initCondition.wait(lock);

	initEngine();
	m_engine->run();
}

void AsepriteRenderHook::initServer()
{
	m_server.bindMessageHandler(std::bind(
		&AsepriteRenderHook::messageHandler,
		this,
		std::placeholders::_1));
}

void AsepriteRenderHook::initEngine()
{
	m_engine->setPostConstructCallback([&] {
		m_server.sendAll("READY");
	});

	m_engine->addTextureDependency(
		{
			{"light",  "Resources/light.png"},
		});
	m_engine->loadTextures();
	m_engine->createMaterial("main material", "albedo", "normal");
	m_engine->createMaterial("light material", "light", "light");
	std::shared_ptr<wrengine::Scene> activeScene = m_engine->getActiveScene();

	// the sprite
	wrengine::Entity spriteEntity = activeScene->createEntity("main");
	auto& spriteRenderComponent = spriteEntity.addComponent<wrengine::SpriteRenderComponent>();
	auto& spriteTransformComponent = spriteEntity.addComponent<wrengine::TransformComponent>();
	spriteEntity.addComponent<wrengine::ScriptComponent>().bind<SpriteController>();
	spriteRenderComponent.material.albedo = m_engine->getTextureByName("albedo");
	spriteRenderComponent.material.normalMap = m_engine->getTextureByName("normal");
	spriteRenderComponent.material.shaderConfig = wrengine::ShaderConfig::NormalMapped;
	spriteTransformComponent.scale = glm::vec3(m_spriteWidth, m_spriteHeight, 1.0f);

	// point light
	wrengine::Entity lightEntity = activeScene->createEntity("light");
	lightEntity.addComponent<wrengine::PointLightComponent>();
	lightEntity.addComponent<wrengine::ScriptComponent>().bind<PointLightController>();
	auto& lightTransformComponent = lightEntity.addComponent<wrengine::TransformComponent>();
	auto& lightRenderComponent = lightEntity.addComponent<wrengine::SpriteRenderComponent>();
	lightTransformComponent.scale = glm::vec3(0.2f);
	lightRenderComponent.material.albedo = m_engine->getTextureByName("light");
	lightRenderComponent.material.normalMap = m_engine->getTextureByName("light");
	lightRenderComponent.material.shaderConfig = wrengine::ShaderConfig::Emissive;

	// the camera
	wrengine::Entity cameraEntity = activeScene->createEntity("camera");
	cameraEntity.addComponent<wrengine::TransformComponent>();
	cameraEntity.addComponent<wrengine::CameraComponent>(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));

	/*TODO add proper constraints on setting the active camera. Direct access as here
	* can lead easily to invalid state.
	*/
	cameraEntity.getComponent<wrengine::CameraComponent>().isActiveCamera = true;
	cameraEntity.getComponent<wrengine::TransformComponent>().translation = glm::vec3( 0.0f );
	cameraEntity.getComponent<wrengine::TransformComponent>().rotation = glm::vec3(0.0f, 0.0f, 1.0f);

	std::shared_ptr<MainWindow> mainWindow = std::make_shared<MainWindow>(m_engine);
	mainWindow->setSprite(spriteEntity);
	mainWindow->setLight(lightEntity);
	m_engine->getUIManager()->pushElement(mainWindow);
}

void AsepriteRenderHook::messageHandler(WebsocketServer::MessageType message)
{
	/**
	* the aseprite lua client will send payload data as a header of 3 ulong types.
	* The first tells us whether the payload is init or post init. The second and
	* third are width/height dimensions in pixels. Payload follows as uint8_t
	* types in sets of 4 RGBA. Sprite albedo valeus and normal map values are
	* packaged sequentially.
	*/
	static int diffuseCount = 0;
	static int normalCount = 0;
	unsigned long* hdr = (unsigned long*)message->get_payload().c_str();

	auto begin = message->get_payload().begin() + 12;
	auto end = message->get_payload().end();

	int width = hdr[1];
	int height = hdr[2];
	std::stringstream ss;

	std::vector<uint8_t> payloadAlbd(begin, begin + (width * height * 4));
	std::vector<uint8_t> payloadNorm(begin + (width * height * 4), end);
	
	if (hdr[0] == 'R')
	{
		std::cout << "recieved sprite update msg" << std::endl;
		m_engine->updateTextureData("albedo", std::move(payloadAlbd));
		m_engine->updateTextureData("normal", std::move(payloadNorm));
	}
	else if (hdr[0] == 'I')
	{
		std::cout << "recieved init msg" << std::endl;
		m_spriteWidth = width;
		m_spriteHeight = height;
		m_engine->loadTexture("albedo", payloadAlbd.data(), width, height, { .filterType = WR_FILTER_NEAREST });
		m_engine->loadTexture("normal", payloadNorm.data(), width, height, { .filterType = WR_FILTER_NEAREST });
		m_initCondition.notify_one();
	}
	std::cout << ss.str();

	std::cout << hdr[0] << "\n";
	std::cout << "width/height: " << hdr[1] << " " << hdr[2] << "\n";
}