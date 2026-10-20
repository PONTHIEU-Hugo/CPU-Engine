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

	m_view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	m_proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
}

void Engine::handleEvent(const SDL_Event& event)
{
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	m_time += 1.0f / 60.0f; // faut faire un vrai delta time

	frameBufferOut.clear(0x000000);
	m_pipeline.ClearDepth();

	Uniforms uniforms;
	uniforms.mvp = glm::mat4(1.0f);

	glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_time, glm::vec3(0, 1, 0));
	uniforms.mvp = m_proj * m_view * model;

	m_pipeline.BindFrameBuffer(&frameBufferOut);
	m_pipeline.BindVertexBuffer(&m_triangle);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
