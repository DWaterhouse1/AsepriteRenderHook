// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook()
{
	wrengine::EngineConfigInfo configInfo{};
	configInfo.height = HEIGHT;
	configInfo.width = WIDTH;
	configInfo.windowName = "Aseprite Render Hook";
	m_engine = std::make_unique<wrengine::Engine>(configInfo);
}

void AsepriteRenderHook::run()
{
	initServer();
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
	m_engine->addTextureDependency(
		{
			{"albedo", "Gemstone_Albedo.png"},
			{"normal", "Gemstone_Normal.png"},
		});
	m_engine->loadTextures();
	m_engine->createMaterial("main material", "albedo", "normal");
	std::shared_ptr<wrengine::Scene> activeScene = m_engine->getActiveScene();
	wrengine::Entity entity = activeScene->createEntity("main entity");
	entity.addComponent<wrengine::SpriteRenderComponent>();
	entity.addComponent<wrengine::Transform2DComponent>();
	
	entity.getComponent<wrengine::SpriteRenderComponent>().material.albedo = m_engine->getTextureByName("albedo");
	entity.getComponent<wrengine::SpriteRenderComponent>().material.normalMap = m_engine->getTextureByName("normal");

	m_engine->getUIManager()->pushElement<DemoWindow>();
}

void AsepriteRenderHook::messageHandler(WebsocketServer::MessageType message)
{
	/**
	* the aseprite lua client will send payload data as a header of 3 ulong types.
	* The first tells us whether the payload is data from the normal map or the
	* sprite albedo. The second and third are width/height dimensions. Payload
	* follows as uint8_t types in sets of 4 RGBA.
	*/
	static int diffuseCount = 0;
	static int normalCount = 0;
	unsigned long* hdr = (unsigned long*)message->get_payload().c_str();

	auto begin = message->get_payload().begin() + 12;
	auto end = message->get_payload().end();
	std::vector<uint8_t> payload(begin, end);

	int width = hdr[1];
	int height = hdr[2];
	std::stringstream ss;
	
	if (hdr[0] == 'D')
	{
		m_engine->updateTextureData("albedo", std::move(payload));
	}
	else if (hdr[0] == 'N')
	{
		m_engine->updateTextureData("normal", std::move(payload));
	}
	std::cout << ss.str();

	std::cout << hdr[0] << "\n";
	std::cout << "width/height: " << hdr[1] << " " << hdr[2] << "\n";
}