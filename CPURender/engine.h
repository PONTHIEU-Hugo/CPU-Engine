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
	void handleEvent();
	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

private:
	Emitter* m_emitter = nullptr;
	Pipeline m_pipeline;
	
	Geometry m_triangle;
	Geometry m_cube;
	Geometry m_car;
	Geometry m_wheelAv;
	Geometry m_wheelAr;
	Geometry m_ground;
	Geometry m_particleVertices;
	
	std::vector<Transform*> m_transforms;
	
	AABB m_triangleBounds;
	AABB m_cubeBounds;
	AABB m_carBounds;
	AABB m_wheelAvBounds;
	AABB m_wheelArBounds;
	AABB m_groundBounds;
	AABB m_sceneBounds;
	
	Texture m_texture;
 
	Camera* m_camera = nullptr;

	// Ombres
	ShadowMap m_shadow;
	std::vector<ShadowCaster> m_shadowCasters;
};

#endif