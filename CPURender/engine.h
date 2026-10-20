#pragma once

#include <vector>

#include "Camera.h"
#include "ParticleSystem.h"
#include "main.h"
#include "core.h"
#include "Texture.h"

union SDL_Event;

struct Engine 
{

	void init();

	void Update();
	void handleEvent(const SDL_Event& event);
	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

private:
	Emitter* m_emitter = nullptr;
	Pipeline m_pipeline;
	std::vector<Vertex> m_triangle;
	std::vector<Vertex> m_cube;
	std::vector<Vertex> m_particleVertices;
	Texture m_texture;

	Camera m_camera;
	vec3  m_camPos   = { 0.0f, 0.0f, 5.0f };
	float m_camYaw = 0.0f; 
	float m_camPitch = 0.0f;
};
