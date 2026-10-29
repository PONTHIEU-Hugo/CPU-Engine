#include "engine.h"

#include <cstdio>

#include "Chrono.h"
#include "MainScene.h"
#include "Shaders.h"
#include "Tests/MaterialScene.h"


void Engine::init()
{
	printf("Init engine \n");
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

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	m_scene->renderFrame(frameBufferOut);
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
