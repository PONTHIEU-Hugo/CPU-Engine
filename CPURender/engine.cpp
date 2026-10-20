#include "engine.h"
#include "Shaders.h"
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
//Students can change whever they want in this file

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = 
	{
		{ {-1.0f, -1.0f, 0.0f}, {255, 0, 0} },
		{ { 1.0f, -1.0f, 0.0f}, {0, 255, 0} },
		{ { 0.0f,  1.0f, 0.0f}, {0, 0, 255} },
	};

	m_pipeline.BindShader(BasicShader);

	m_view = lookAt(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));
	m_proj = perspective(radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	
	m_emitter = new Emitter({0.0f, 0.0f, 0.0f}, 0);
	m_emitter->Init();
}

void Engine::Update(float _dt)
{
	if (m_emitter != nullptr)
		m_emitter->Update(_dt);
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
	m_time += 1.0f / 60.0f; // faut faire un vrai delta time

	frameBufferOut.clear(0x000000);
	m_pipeline.ClearDepth();
	m_pipeline.BindFrameBuffer(&frameBufferOut);

	Uniforms uniforms;
	mat4 model = rotate(mat4(1.0f), m_time, vec3(0, 1, 0));
	uniforms.mvp = m_proj * m_view * model;
	m_pipeline.BindVertexBuffer(&m_triangle);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();

	m_particleVertices.clear();
	if (m_emitter)
		m_emitter->Render(m_particleVertices);

	uniforms.mvp = m_proj * m_view;
	m_pipeline.BindVertexBuffer(&m_particleVertices);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
}

void Engine::destroy()
{
	delete m_emitter;
	printf("Killing engine \n");
}
