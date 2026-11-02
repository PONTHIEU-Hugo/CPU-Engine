#include "engine.h"

#include <cstdio>

#include "Chrono.h"
#include "MainScene.h"
#include "Shaders.h"
#include "Tests/AnimatorScene.h"
#include "Tests/MaterialScene.h"


void Engine::init()
{
	printf("Init engine \n");
	m_renderer.Init(2048);
	m_renderer.showShadowDebug = false;
	m_scene = new MainScene();
	m_scene->Init();
}

void Engine::Update()
{
	const float dt = static_cast<float>(CHRONO->GetDeltaTime());
	m_scene->Update(dt);
}

void Engine::handleEvent()
{
}

void Engine::RenderFrame(FinalFrameBuffer& frameBufferOut)
{
	m_renderer.Render(*m_scene, frameBufferOut);
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
