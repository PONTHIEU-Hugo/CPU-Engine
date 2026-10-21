#pragma once

#include <vector>

#include "ParticleSystem.h"
#include "main.h"
#include "core.h"
#include "TerrainGenerator.h"
//Students can change whetever they want in this file

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
	TerrainGenerator* m_Terrain = nullptr;

	Pipeline m_pipeline;
	std::vector<Vertex> m_triangle;
	std::vector<Vertex> m_particleVertices;

	mat4 m_view{ 1.0f };
	mat4 m_proj{ 1.0f };
};
