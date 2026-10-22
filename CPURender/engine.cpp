#include "engine.h"
#include "Shaders.h"
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Chrono.h"
#include "GeometryFactory.hpp"

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = GeometryFactory::BuildTriangle();
	m_cube = GeometryFactory::BuildCube();

	// m_obj = GeometryFactory::BuildTriangle();
	// m_obj = GeometryFactory::BuildPyramid();
	// m_obj = GeometryFactory::BuildCube();
	// m_obj = GeometryFactory::BuildIcosphere(3);         
	// m_obj = GeometryFactory::BuildIcosphere(4, 1.0f);
	// m_obj = GeometryFactory::BuildUVSphere(32, 16);        
	// m_obj = GeometryFactory::BuildUVSphere(64, 32, 2.0f);
	// m_obj = GeometryFactory::BuildCircle(32, 0.0f, true);
	// m_obj = GeometryFactory::BuildCircle(32, 0.0f, false);
	// m_obj = GeometryFactory::BuildCylinder(32);
	// m_obj = GeometryFactory::BuildDonut(0.5f, 0.2f, 48, 24);
	
	//m_obj = GeometryFactory::LoadGeometry("assets/tuture.obj");
	
	
	m_texture.Load("Assets/mael.png");

	m_triangleBounds = ComputeAABB(m_triangle);
	m_cubeBounds = ComputeAABB(m_cube);

	m_pipeline.BindShader(BasicShader);

	float aspect = float(WIDTH) / float(HEIGHT);
	
	// m_camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_camera.SetPerspective(radians(45.0f), aspect, 0.1f, 100.0f);
	
	m_emitter = new Emitter({0.0f, 0.0f, 0.0f}, 1000);
	m_emitter->Init();
}

void Engine::Update()
{
	float dt = static_cast<float>(CHRONO->GetDeltaTime());
	m_camera.HandleEvent(dt);
	if (m_emitter != nullptr)
		m_emitter->Update();
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if ( event.key.key == SDLK_P )
        {
            m_emitter->m_active = !m_emitter->m_active;
        }
    }
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	frameBufferOut.clear(0x000000);
	m_pipeline.ClearDepth();
	m_pipeline.BindFrameBuffer(&frameBufferOut);

	Uniforms uniforms;
	uniforms.mvp = m_camera.GetVPMatrix();
	uniforms.lightDirection = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
	uniforms.ambient = 0.2f;

	Frustum frustum = Frustum::FromMatrix(m_camera.GetVPMatrix());

	//Triangle
	if (frustum.Intersects(m_triangleBounds))
	{
		m_pipeline.BindShader(BasicShader);
		m_pipeline.BindVertexBuffer(&m_triangle);
		m_pipeline.BindUniforms(uniforms);
		//m_pipeline.Draw();
	}

	// Cube
	if (frustum.Intersects(m_cubeBounds))
	{
		m_pipeline.BindShader(LambertShader);
		m_pipeline.BindVertexBuffer(&m_cube);
		m_pipeline.BindUniforms(uniforms);
		m_pipeline.Draw();
	}

	m_pipeline.BindShader(BasicShader);

	//m_particleVertices.clear();
	//if (m_emitter)
	//	m_emitter->Render(m_particleVertices);

	//uniforms.mvp = m_camera.GetVPMatrix();
	//m_pipeline.BindVertexBuffer(&m_particleVertices);
	//m_pipeline.BindUniforms(uniforms);
	//m_pipeline.Draw();
}

void Engine::destroy()
{
	delete m_emitter;
	printf("Killing engine \n");
}
