#pragma once

#include "Wrengine.h"

//std
#include <vector>
#include <array>

constexpr std::array<float, 3> DEFAULT_CLEAR = { 0.1f, 0.1f, 0.1f };

class MainWindow : public wrengine::UIElement
{
public:
	MainWindow(std::shared_ptr<wrengine::Engine> engine) :
		m_engine{ engine }
	{}

	virtual void onAttach() override;
	virtual void onDetatch() override;

	void setSprite(wrengine::Entity sprite) { m_mainSprite = sprite; }
	void setLight(wrengine::Entity light) { m_light = light; }

protected:
	virtual void onUIRender() override;

private:
	std::shared_ptr<wrengine::Engine> m_engine;

	// normal coords
	bool m_invertNormalsX = false;
	bool m_invertNormalsY = false;
	bool m_invertNormalsZ = false;

	// clear colour
	bool m_useDefaultClearColor = true;
	std::vector<float> m_clearColor =
	{
		DEFAULT_CLEAR[0],
		DEFAULT_CLEAR[1],
		DEFAULT_CLEAR[2]
	};

	// sprite controls
	wrengine::Entity m_mainSprite;
	wrengine::Entity m_light;
	wrengine::TransformComponent* m_spriteTransform = nullptr;
	wrengine::TransformComponent* m_lightTransform = nullptr;
};