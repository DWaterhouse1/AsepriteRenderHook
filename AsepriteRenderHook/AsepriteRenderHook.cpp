﻿// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook() {}

void AsepriteRenderHook::run()
{
	m_engine.addTextureDependency({
			{"texture", "landscape.png"},
		});
	m_engine.run();
}