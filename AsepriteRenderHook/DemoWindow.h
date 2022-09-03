#pragma once

#include "UserInterface.h"

class DemoWindow : public wrengine::UIElement
{
public:
	DemoWindow() {}
	virtual void onUIRender() override;

private:
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};