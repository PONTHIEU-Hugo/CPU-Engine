#pragma once

#include <vector>

#include "Camera.h"
#include "ParticleSystem.h"
#include "main.h"
#include "core.h"
#include "Texture.h"
#include "Frustum.h"

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
	std::vector<Vertex> m_obj;
	std::vector<Vertex> m_particleVertices;
	Texture m_texture;
	AABB m_triangleBounds;
	AABB m_cubeBounds;
	AABB m_objBounds;

	Camera m_camera;
};
