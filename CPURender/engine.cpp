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

	// m_obj = GeometryFactory::BuildTriangle();
	// m_obj = GeometryFactory::BuildPyramid();
	// m_obj = GeometryFactory::BuildCube();
	// m_obj = GeometryFactory::BuildIcosphere(3);         
	// m_obj = GeometryFactory::BuildIcosphere(1, 1.0f);
	// m_obj = GeometryFactory::BuildUVSphere(8, 4);        
	// m_obj = GeometryFactory::BuildUVSphere(64, 32, 2.0f);
	// m_obj = GeometryFactory::BuildCircle(32, 0.0f, true);
	// m_obj = GeometryFactory::BuildCircle(32, 0.0f, false);
	// m_obj = GeometryFactory::BuildCylinder(32);
	 // m_obj = GeometryFactory::BuildDonut(0.5f, 0.2f, 48, 24);
	
	// Ressources
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
