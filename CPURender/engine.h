#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "core.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "main.h"
#include "Texture.h"
#include "Frustum.h"
#include "ShadowMap.h"

#define SHADOW_DEBUG 1



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
	std::vector<Vertex> m_ground;
	std::vector<Vertex> m_particleVertices;
	Texture m_texture;
	AABB m_triangleBounds;
	AABB m_cubeBounds;
	AABB m_objBounds;
	AABB m_groundBounds;
	AABB m_sceneBounds;

	Camera m_camera;

	// Ombres
	ShadowMap m_shadow;
	std::vector<ShadowCaster> m_shadowCasters;
	
	#if SHADOW_DEBUG
		bool m_forceShadowEveryFrame = false;
		double m_msShadow = 0.0;
		double m_msMain = 0.0;
		int m_profFrames = 0;
	#endif
};

#endif