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
	m_cube.transform.SetPosition({0.0f, 1.0f, 0.0f});

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
	
	m_car = GeometryFactory::LoadGeometry("Assets/car.obj");
	m_car.transform.SetPosition({-10.0f, 0.0f, 0.0f});
	// m_car.transform.SetParent(&m_cube.transform);
	m_wheelAv = GeometryFactory::LoadGeometry("Assets/wheelAv.obj");
	m_wheelAv.transform.SetPosition({0.0f, 1.05f, -5.5f});
	m_wheelAv.transform.SetParent(&m_car.transform);
	m_wheelAr = GeometryFactory::LoadGeometry("Assets/wheelAr.obj");
	m_wheelAr.transform.SetPosition({0.0f, 1.1f, 5.0f});
	m_wheelAr.transform.SetParent(&m_car.transform);
	
	m_geo.push_back(&m_car);
	m_geo.push_back(&m_wheelAv);
	m_geo.push_back(&m_wheelAr);
	m_geo.push_back(&m_cube);
	m_geo.push_back(&m_triangle);
	m_geo.push_back(&m_ground);
	
	m_texture.Load("Assets/vivien.png");

	m_triangleBounds = ComputeAABB(m_triangle);
	m_cubeBounds = ComputeAABB(m_cube);
	m_carBounds = ComputeAABB(m_car);
	m_wheelAvBounds = ComputeAABB(m_wheelAv);
	m_wheelArBounds = ComputeAABB(m_wheelAr);

	float r = glm::length(m_carBounds.max - m_carBounds.min) * 0.5f;
	m_ground = BuildGround(m_carBounds.min.y, r * 5.0f);
	m_groundBounds = ComputeAABB(m_ground);

	m_sceneBounds.min = glm::min(m_carBounds.min, 10.0f);
	m_sceneBounds.max = glm::max(m_carBounds.max, 10.0f);

	float aspect = float(WIDTH) / float(HEIGHT);
	
	m_camera = new Camera({0.0f, 4.5f, 3.0f}, &m_car.transform);
	// m_camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_camera->SetPerspective(radians(45.0f), aspect, 0.1f, 100.0f);
	
	m_shadow.Init(512);
	m_shadow.SetFollowMode(vec3(-0.5f, -1.0f, -0.3f), 50.0f, 30.0f);
	m_shadow.Follow(m_camera->GetPosition());
	m_shadowCasters = { 
		{ &m_car, m_carBounds }, 
		{ &m_wheelAv, m_wheelAvBounds }, 
		{ &m_wheelAr, m_wheelArBounds }, 
		{ &m_cube, m_cubeBounds } 
	};

	m_pipeline.BindShader(BasicShader);
	
	m_emitter = new Emitter({0.0f, 0.0f, 0.0f}, 1000);
	m_emitter->Init();
}

void Engine::Update()
{
	float dt = static_cast<float>(CHRONO->GetDeltaTime());
	m_camera->HandleEvent(dt);
	if (m_emitter != nullptr)
		m_emitter->Update();
	
	vec3 cam = m_camera->GetPosition();
	m_shadow.Follow(vec3(cam.x, m_groundBounds.min.y, cam.z));
}

void Engine::handleEvent()
{
	const bool* keys = SDL_GetKeyboardState(nullptr);
	
	if (keys[SDL_SCANCODE_R])
	{
		mat4 rot = rotate(mat4(1.0f), radians(15.0f), vec3(0, 1, 0));
		m_shadow.SetFollowMode(vec3(rot * vec4(m_shadow.LightDirection(), 0.0f)), 50.0f, 30.0f);
	}
	
	float dt = CHRONO->GetDeltaTime();
	float speed = 20.0f;
	float speedRot = 10.0f;
	float speedSca = 0.2f;
	
	Geometry* currentObj = &m_car;
	
	if (keys[SDL_SCANCODE_KP_8])
	{
		currentObj->transform.Move(currentObj->transform.GetForward() * (speed * dt));
		m_wheelAr.transform.AddYPR({0.0f, -speedRot * dt, 0.0f});
		m_wheelAv.transform.AddYPR({0.0f, -speedRot * dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_5])
	{
		currentObj->transform.Move(currentObj->transform.GetForward() * (-speed * dt));
		m_wheelAr.transform.AddYPR({0.0f, speedRot * 0.5 * dt, 0.0f});
		m_wheelAv.transform.AddYPR({0.0f, speedRot * 0.5* dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_1])
		currentObj->transform.Move(currentObj->transform.GetUp() * (-speed * dt));
	if (keys[SDL_SCANCODE_KP_3])
		currentObj->transform.Move(currentObj->transform.GetUp() * (speed * dt));
	
	if (keys[SDL_SCANCODE_KP_4])
		currentObj->transform.AddYPR({speedRot * dt, 0.0f, 0.0f});
	if (keys[SDL_SCANCODE_KP_6])
		currentObj->transform.AddYPR({-speedRot * dt, 0.0f, 0.0f});
	
	if (keys[SDL_SCANCODE_KP_PLUS])
		currentObj->transform.AddScale(speedSca * dt);
	if (keys[SDL_SCANCODE_KP_MINUS])
		currentObj->transform.AddScale(-speedSca * dt);
	
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	AABB carWorld  = TransformAABB(m_carBounds,  m_car.transform.GetWorldMatrix());
	AABB wheelAvWorld  = TransformAABB(m_wheelAvBounds,  m_wheelAv.transform.GetWorldMatrix());
	AABB wheelArWorld  = TransformAABB(m_wheelArBounds,  m_wheelAr.transform.GetWorldMatrix());
	AABB cubeWorld = TransformAABB(m_cubeBounds, m_cube.transform.GetWorldMatrix());
	
	m_shadowCasters[0].bounds = carWorld;
	m_shadowCasters[1].bounds = wheelAvWorld;
	m_shadowCasters[2].bounds = wheelArWorld;
	m_shadowCasters[3].bounds = cubeWorld;
	
	Uniforms uniforms;
	uniforms.mvp     = m_camera->GetVPMatrix();
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

		Frustum frustum = Frustum::FromMatrix(m_camera->GetVPMatrix());

		
		if (frustum.Intersects(cubeWorld))
		{
			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_cube);
			m_pipeline.BindUniforms(uniforms);
			m_pipeline.Draw(m_cube.transform.GetWorldMatrix());
		}

		if (frustum.Intersects(carWorld))
		{
			Uniforms objUniforms = uniforms;
			if (m_texture.IsValid())
				objUniforms.texture = &m_texture;

			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_car);
			m_pipeline.BindUniforms(objUniforms);
			m_pipeline.Draw(m_car.transform.GetWorldMatrix());
		}
		
		if (frustum.Intersects(wheelAvWorld))
		{
			Uniforms objUniforms = uniforms;
			if (m_texture.IsValid())
				objUniforms.texture = &m_texture;

			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_wheelAv);
			m_pipeline.BindUniforms(objUniforms);
			m_pipeline.Draw(m_wheelAv.transform.GetWorldMatrix());
		}
		if (frustum.Intersects(wheelArWorld))
		{
			Uniforms objUniforms = uniforms;
			if (m_texture.IsValid())
				objUniforms.texture = &m_texture;

			m_pipeline.BindShader(ShadowLambertShader);
			m_pipeline.BindVertexBuffer(&m_wheelAr);
			m_pipeline.BindUniforms(objUniforms);
			m_pipeline.Draw(m_wheelAr.transform.GetWorldMatrix());
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