#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "core.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "main.h"
#include "Texture.h"
#include "Frustum.h"
#include "ShadowMap.h"

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
	
	Geometry m_triangle;
	Geometry m_cube;
	Geometry m_obj;
	Geometry m_ground;
	Geometry m_particleVertices;
	
	AABB m_triangleBounds;
	AABB m_cubeBounds;
	AABB m_objBounds;
	AABB m_groundBounds;
	AABB m_sceneBounds;
	
	Texture m_texture;

	Camera m_camera;

	// Ombres
	ShadowMap m_shadow;
	std::vector<ShadowCaster> m_shadowCasters;
};

#endif