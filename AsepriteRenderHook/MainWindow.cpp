#include "MainWindow.h"
#include "imgui.h"

void MainWindow::onAttach()
{
	m_spriteTransform = &m_mainSprite.getComponent<wrengine::TransformComponent>();
	m_lightTransform = &m_light.getComponent<wrengine::TransformComponent>();
}

void MainWindow::onDetatch()
{
	m_spriteTransform = nullptr;
	m_spriteTransform = nullptr;
}

void MainWindow::onUIRender()
{
	if (!ImGui::Begin("Controls"))
	{
		// take an early out if the window is collapsed
		ImGui::End();
		return;
	}
	ImGui::Text("Invert normal map coords:");

	bool normalsDirty = false;
	normalsDirty |= ImGui::Checkbox("x", &m_invertNormalsX);
	ImGui::SameLine();
	normalsDirty |= ImGui::Checkbox("y", &m_invertNormalsY);
	ImGui::SameLine();
	normalsDirty |= ImGui::Checkbox("z", &m_invertNormalsZ);
	ImGui::Spacing();

	if (normalsDirty)
	{
		float x, y, z;
		x = m_invertNormalsX ? -1.0f : 1.0f;
		y = m_invertNormalsY ? -1.0f : 1.0f;
		z = m_invertNormalsZ ? -1.0f : 1.0f;
		m_engine->setNormalCoordinateScales(x, y, z);
	}

	if (ImGui::Checkbox("Use default clear colour", &m_useDefaultClearColor))
	{
		m_useDefaultClearColor ?
			m_engine->setClearColor(DEFAULT_CLEAR[0], DEFAULT_CLEAR[1], DEFAULT_CLEAR[2]) :
			m_engine->setClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2]);
	}

	if (!m_useDefaultClearColor)
	{
		if (ImGui::ColorEdit3("Clear Colour", m_clearColor.data()))
		{
			m_engine->setClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2]);
		}
	}

	ImGui::Separator();


	if (ImGui::CollapsingHeader("Position Controls"))
	{
		if (ImGui::TreeNode("Sprite"))
		{
			static float spriteX = 0.0f;
			static float spriteY = 0.0f;
			bool spritePosDirty = false;
			ImGui::Text("Sprite position");
			spritePosDirty |= ImGui::SliderFloat("x", &spriteX, -500.0f, 500.0f);
			spritePosDirty |= ImGui::SliderFloat("y", &spriteY, -500.0f, 500.0f);

			if (spritePosDirty)
			{
				m_spriteTransform->translation.x = spriteX;
				m_spriteTransform->translation.y = spriteY;
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Light"))
		{
			static float lightX = 0.0f;
			static float lightY = 0.0f;
			bool spritePosDirty = false;
			ImGui::Text("Sprite position");
			spritePosDirty |= ImGui::SliderFloat("x", &lightX, -700.0f, 700.0f);
			spritePosDirty |= ImGui::SliderFloat("y", &lightY, -700.0f, 700.0f);

			if (spritePosDirty)
			{
				// update light position
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
}