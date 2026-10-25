#ifndef ENGINE_CPP_INCLUDED
#define ENGINE_CPP_INCLUDED

#include "engine.h"

#include <iostream>

#include "Shaders.h"
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Chrono.h"
#include "GeometryFactory.hpp"

// Quad horizontal qui reçoit les ombres
static std::vector<Vertex> BuildGround(float y, float half)
{
	auto make = [&](float x, float z)
	{
		Vertex v;
		v.position = { x, y, z };
		v.normal   = { 0.0f, 1.0f, 0.0f };
		v.color    = { 180.0f, 180.0f, 180.0f };
		return v;
	};
	Vertex p0 = make(-half, half), p1 = make(half, half), p2 = make(half, -half), p3 = make(-half, -half);
	return { p0, p1, p2, p0, p2, p3 };
}

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = GeometryFactory::BuildTriangle();
	m_cube = GeometryFactory::BuildCube();

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
	
	m_texture.Load("Assets/vivien.png");

	m_triangleBounds = ComputeAABB(m_triangle);
	m_cubeBounds = ComputeAABB(m_cube);
	m_objBounds = ComputeAABB(m_obj);

	float r = glm::length(m_objBounds.max - m_objBounds.min) * 0.5f;
	m_ground = BuildGround(m_objBounds.min.y, r * 1.2f);
	m_groundBounds = ComputeAABB(m_ground);

	m_sceneBounds.min = glm::min(m_objBounds.min, m_groundBounds.min);
	m_sceneBounds.max = glm::max(m_objBounds.max, m_groundBounds.max);

	m_shadow.Init(256);
	m_shadow.SetLight(vec3(-0.5f, -1.0f, -0.3f), m_sceneBounds);
	m_shadowCasters = { { &m_obj, m_objBounds } };

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
	if (event.type != SDL_EVENT_KEY_DOWN)
		return;

	if (event.key.key == SDLK_R)
	{
		mat4 rot = rotate(mat4(1.0f), radians(15.0f), vec3(0, 1, 0));
		m_shadow.SetLight(vec3(rot * vec4(m_shadow.LightDirection(), 0.0f)), m_sceneBounds);
	}
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	Uniforms uniforms;
	uniforms.mvp     = m_camera.GetVPMatrix();
	uniforms.ambient = 0.2f;
	m_shadow.ApplyTo(uniforms);   // lumière, matrice, carte, biais

	// Passe 1 : depuis la lumière, seulement si la carte est invalide
	{
		m_shadow.Update(m_pipeline, m_shadowCasters);
	}

	// Passe 2 : depuis la caméra
	{
		frameBufferOut.clear(0x000000);
		m_pipeline.BindFrameBuffer(&frameBufferOut);   // rebascule sur l'écran
		m_pipeline.ClearDepth();
		m_pipeline.SetCullMode(CullMode::Back);

		Frustum frustum = Frustum::FromMatrix(m_camera.GetVPMatrix());

		
		// Cube (désactivé)
		if (frustum.Intersects(m_cubeBounds))
		{
			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_cube);
			m_pipeline.BindUniforms(uniforms);
			m_pipeline.Draw();
		}

		// Objet : reçoit les ombres
		if (frustum.Intersects(m_objBounds))
		{
			Uniforms objUniforms = uniforms;
			if (m_texture.IsValid())
				objUniforms.texture = &m_texture;

			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_obj);
			m_pipeline.BindUniforms(objUniforms);
			m_pipeline.Draw();
		}

		// Sol : face unique, donc sans culling
		if (frustum.Intersects(m_groundBounds))
		{
			m_pipeline.SetCullMode(CullMode::None);
			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_ground);
			m_pipeline.BindUniforms(uniforms);
			m_pipeline.Draw();
			m_pipeline.SetCullMode(CullMode::Back);
		}

		// Particules (désactivées)
		//m_particleVertices.clear();
		//if (m_emitter)
		//	m_emitter->Render(m_particleVertices);
		//m_pipeline.BindShader(BasicShader);
		//m_pipeline.BindVertexBuffer(&m_particleVertices);
		//m_pipeline.BindUniforms(uniforms);
		//m_pipeline.Draw();
	}
	
	m_shadow.DrawDebug(frameBufferOut, 64);
}

void Engine::destroy()
{
	delete m_emitter;
	printf("Killing engine \n");
}

#endif