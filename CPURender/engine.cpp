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
static Geometry BuildGround(float y, float half)
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
	Geometry geometry;
	geometry.vertices = { p0, p1, p2, p0, p2, p3 };
	return geometry;
}

static AABB TransformAABB(const AABB& b, const mat4& m)
{
	AABB r;
	r.min = vec3(1e30f);
	r.max = vec3(-1e30f);
	for (int i = 0; i < 8; ++i)
	{
		vec3 corner((i & 1) ? b.max.x : b.min.x,
					(i & 2) ? b.max.y : b.min.y,
					(i & 4) ? b.max.z : b.min.z);
		vec3 w = vec3(m * vec4(corner, 1.0f));
		r.min = glm::min(r.min, w);
		r.max = glm::max(r.max, w);
	}
	return r;
}

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = GeometryFactory::BuildTriangle();
	m_cube = GeometryFactory::BuildCube();
	m_cube.transform.SetPosition({0.0f, 0.5f, 0.0f});

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
	
	m_obj = GeometryFactory::LoadGeometry("Assets/tuture.obj");
	
	m_texture.Load("Assets/vivien.png");

	m_triangleBounds = ComputeAABB(m_triangle);
	m_cubeBounds = ComputeAABB(m_cube);
	m_objBounds = ComputeAABB(m_obj);

	float r = glm::length(m_objBounds.max - m_objBounds.min) * 0.5f;
	m_ground = BuildGround(m_objBounds.min.y, r * 5.0f);
	m_groundBounds = ComputeAABB(m_ground);

	m_sceneBounds.min = glm::min(m_objBounds.min, 10.0f);
	m_sceneBounds.max = glm::max(m_objBounds.max, 10.0f);

	m_shadow.Init(512);
	m_shadow.SetFollowMode(vec3(-0.5f, -1.0f, -0.3f), 50.0f, 30.0f);
	m_shadow.Follow(m_camera.GetCameraPos());
	m_shadowCasters = { { &m_obj, m_objBounds }, { &m_cube, m_cubeBounds } };

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
	
	vec3 cam = m_camera.GetCameraPos();
	m_shadow.Follow(vec3(cam.x, m_groundBounds.min.y, cam.z));
}

void Engine::handleEvent(const SDL_Event& event)
{
	if (event.key.key == SDLK_R)
	{
		mat4 rot = rotate(mat4(1.0f), radians(15.0f), vec3(0, 1, 0));
		m_shadow.SetFollowMode(vec3(rot * vec4(m_shadow.LightDirection(), 0.0f)), 50.0f, 30.0f);
	}
	
	float dt = CHRONO->GetDeltaTime();
	float speed = 15.0f;
	if (event.key.key == SDLK_KP_8)
		m_obj.transform.Move(vec3(0.0f, 0.0f, speed * dt));
	if (event.key.key == SDLK_KP_5)
		m_obj.transform.Move(vec3(0.0f, 0.0f, -speed * dt));
	if (event.key.key == SDLK_KP_4)
		m_obj.transform.Move(vec3(-speed * dt, 0.0f, 0.0f));
	if (event.key.key == SDLK_KP_6)
		m_obj.transform.Move(vec3(speed * dt, 0.0f, 0.0f));
	
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	AABB objWorld  = TransformAABB(m_objBounds,  m_obj.transform.GetWorldMatrix());
	AABB cubeWorld = TransformAABB(m_cubeBounds, m_cube.transform.GetWorldMatrix());
	m_shadowCasters[0].bounds = objWorld;
	m_shadowCasters[1].bounds = cubeWorld;
	
	Uniforms uniforms;
	uniforms.mvp     = m_camera.GetVPMatrix();
	uniforms.ambient = 0.2f;
	m_shadow.ApplyTo(uniforms);

	// Passe 1
	{
		m_shadow.Update(m_pipeline, m_shadowCasters);
	}

	// Passe 2
	{
		frameBufferOut.clear(0x000000);
		m_pipeline.BindFrameBuffer(&frameBufferOut);
		m_pipeline.ClearDepth();
		m_pipeline.SetCullMode(CullMode::Back);

		Frustum frustum = Frustum::FromMatrix(m_camera.GetVPMatrix());

		
		if (frustum.Intersects(cubeWorld))
		{
			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_cube);
			m_pipeline.BindUniforms(uniforms);
			m_pipeline.Draw(m_cube.transform.GetWorldMatrix());
		}

		if (frustum.Intersects(objWorld))
		{
			Uniforms objUniforms = uniforms;
			if (m_texture.IsValid())
				objUniforms.texture = &m_texture;

			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_obj);
			m_pipeline.BindUniforms(objUniforms);
			m_pipeline.Draw(m_obj.transform.GetWorldMatrix());
		}

		if (frustum.Intersects(m_groundBounds))
		{
			m_pipeline.SetCullMode(CullMode::None);
			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_ground);
			m_pipeline.BindUniforms(uniforms);
			m_pipeline.Draw(m_ground.transform.GetWorldMatrix());
			m_pipeline.SetCullMode(CullMode::Back);
		}
	}
	
	m_shadow.DrawDebug(frameBufferOut, 64);
}

void Engine::destroy()
{
	delete m_emitter;
	printf("Killing engine \n");
}

#endif