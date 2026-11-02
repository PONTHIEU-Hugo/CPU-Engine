#ifndef MAIN_SCENE_CPP_INCLUDED
#define MAIN_SCENE_CPP_INCLUDED

#include "MainScene.h"

#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shaders.h"
#include "GeometryFactory.hpp"

static Mesh BuildGround(float y, float half)
{
	auto make = [&](float x, float z)
	{
		Vertex v;
		v.position = { x, y, z };
		v.normal   = { 0.0f, 1.0f, 0.0f };
		v.color    = { 180.0f, 180.0f, 180.0f, 255.0f };
		return v;
	};
	Vertex p0 = make(-half, half), p1 = make(half, half), p2 = make(half, -half), p3 = make(-half, -half);

	Mesh mesh;
	mesh.vertices = { p0, p1, p2, p0, p2, p3 };
	mesh.ComputeBounds();
	return mesh;
}

void MainScene::Init()
{
    m_cubeMesh = GeometryFactory::BuildCube();
	m_carMesh = GeometryFactory::LoadGeometry("Assets/car.obj");
	m_wheelArMesh = GeometryFactory::LoadGeometry("Assets/wheelAr.obj");
	m_wheelAvMesh = GeometryFactory::LoadGeometry("Assets/wheelAv.obj");
	
	m_carTexture.Load("Assets/vivien.png");
	m_cubeTexture.Load("Assets/max.png");

	const AABB& carBounds = m_carMesh.localBounds;
	float carRadius = glm::length(carBounds.max - carBounds.min) * 0.5f;
	m_groundMesh = BuildGround(carBounds.min.y, carRadius * 5.0f);

	// Materiaux
	m_defaultMaterial.shader = &ShadowLambertShader;
	
	m_cubeMaterial = m_defaultMaterial;
	m_cubeMaterial.baseColor.a = 255;
	m_cubeMaterial.transparent = false;
	// if (m_cubeTexture.IsValid())
	// 	m_cubeMaterial.texture = &m_cubeTexture;

	m_carMaterial = m_defaultMaterial;
	m_carMaterial.baseColor.a = 255 / 2;
	m_carMaterial.transparent = true;
	if (m_carTexture.IsValid()) 
		m_carMaterial.texture = &m_carTexture;

	m_groundMaterial      = m_defaultMaterial;
	m_groundMaterial.cull = CullMode::None;

	m_defaultMaterial.specular  = 0.8f;  
	m_defaultMaterial.roughness = 0.3f;

	m_carMaterial.specular      = 0.5f;
	m_carMaterial.roughness     = 0.4f;

	// Scene 
	//ObjectId cube = Add(m_cubeMesh, m_defaultMaterial);
	//Get(cube).transform.SetPosition({ 0.0f, 0.5f, 0.0f });

	m_car = Add(m_carMesh, m_carMaterial, true,
		new Transform({-10.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	m_wheelAr = Add(m_wheelArMesh, m_carMaterial, true,
			new Transform({0.0f, 1.1f, 5.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	m_wheelAv = Add(m_wheelAvMesh, m_carMaterial, true,
	 		new Transform({0.0f, 1.05f, -5.5f}, {1,0,0,0}, glm::vec3(1.0f)));
	Add(m_cubeMesh, m_cubeMaterial, true,
	 		new Transform({-7.0f, 1.0f, 15.0f}, {1,0,0,0}, glm::vec3(5.0f)));
	Add(m_groundMesh, m_groundMaterial, false);
	
	Transform& carT = Get(m_car).transform;
	Get(m_wheelAr).transform.SetParent(&carT);
	Get(m_wheelAv).transform.SetParent(&carT);
	
	float aspect = float(WIDTH) / float(HEIGHT);
	camera = new Camera({0.0f, 4.5f, 3.0f}, nullptr);
	// camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	m_transforms.push_back(&camera->transform);

	sun.direction         = glm::vec3(-0.5f, -1.0f, -0.3f);
	sun.ambient           = 0.2f;
	sun.shadowRadius      = 50.0f;
	sun.shadowCasterExtra = 30.0f;
	sun.shadowFocus       = camera->GetPosition();

	m_Terrain.GenerateHeighmap(glm::vec2(50, 50), HeighMapType::_OCTAVE_PERLIN_2D);	
	//Get(Add(m_Terrain.GetMesh(), m_Terrain.GetMaterial())).transform.SetPosition({ 0, -5, 0 });

	// Systèmes
	LoadSkybox();
	
	m_snow.groundHeight = m_groundMesh.localBounds.min.y;

	m_smoke.Init();
	m_snow.Init();
	m_sparkle.Init();

	// Ordre de dessin : fumee et neige (alpha) puis etincelles (additif).
	AddEmitter(m_smoke);
	AddEmitter(m_snow);
	AddEmitter(m_sparkle);
}

void MainScene::Update(float _dt)
{
	camera->HandleEvent(_dt);
	HandleEvent(_dt);

	const glm::vec3 exhaustLocalOffset(1.7f, 1.3f, 9.2f);
	const glm::mat4& carWorld = Get(m_car).transform.GetWorldMatrix();
	m_smoke.SetOrigin(glm::vec3(carWorld * glm::vec4(exhaustLocalOffset, 1.0f)));
	m_smoke.Update(_dt);

	const glm::vec3 roofLocalOffset(0.0f, 4.0f, 0.0f);
	m_sparkle.SetOrigin(glm::vec3(carWorld * glm::vec4(roofLocalOffset, 1.0f)));
	m_sparkle.Update(_dt);

	glm::vec3 cam = camera->GetPosition();
	m_snow.SetOrigin(cam + glm::vec3(0.0f, 20.0f, 0.0f));
	m_snow.Update(_dt);

	sun.shadowFocus = glm::vec3(cam.x, m_groundMesh.localBounds.min.y, cam.z);
	Scene::Update(_dt);
}

void MainScene::LoadSkybox()
{
    skybox.faces[0].Load("Assets/right.jpg");
    skybox.faces[1].Load("Assets/left.jpg");
    skybox.faces[2].Load("Assets/top.jpg");
    skybox.faces[3].Load("Assets/bottom.jpg");
    skybox.faces[4].Load("Assets/front.jpg");
    skybox.faces[5].Load("Assets/back.jpg");
}

void MainScene::HandleEvent(float _dt)
{
	const bool* keys = SDL_GetKeyboardState(nullptr);
	
	bool pDown = keys[SDL_SCANCODE_P];
	if (pDown && m_pWasDown == false)
	{
		bool active = m_smoke.IsActive() == false;
		m_smoke.SetActive(active);
		m_snow.SetActive(active);
		m_sparkle.SetActive(active);
	}
	m_pWasDown = pDown;

	bool bDown = keys[SDL_SCANCODE_B];
	if (bDown && m_bWasDown == false)
		m_sparkle.Burst(200);
	m_bWasDown = bDown;
	
	if (keys[SDL_SCANCODE_R])
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
		sun.direction = glm::vec3(rot * glm::vec4(sun.direction, 0.0f));
	}
	
	float dt = _dt;
	float speed = 20.0f;
	float speedRot = 10.0f;
	float speedSca = 0.2f;
	
	ObjectId currentObj = m_car;
	
	if (keys[SDL_SCANCODE_KP_8])
	{
		Get(currentObj).transform.Move(Get(currentObj).transform.GetForward() * (speed * dt));
		if ( m_wheelAr != -1) Get(m_wheelAr).transform.AddYPR({0.0f, -speedRot * dt, 0.0f});
		if ( m_wheelAv != -1) Get(m_wheelAv).transform.AddYPR({0.0f, -speedRot * dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_5])
	{
		Get(currentObj).transform.Move(Get(currentObj).transform.GetForward() * (-speed * dt));
		if ( m_wheelAr != -1) Get(m_wheelAr).transform.AddYPR({0.0f, speedRot * dt, 0.0f});
		if ( m_wheelAv != -1) Get(m_wheelAv).transform.AddYPR({0.0f, speedRot * dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_1])
		Get(currentObj).transform.Move(Get(currentObj).transform.GetUp() * (-speed * dt));
	if (keys[SDL_SCANCODE_KP_3])
		Get(currentObj).transform.Move(Get(currentObj).transform.GetUp() * (speed * dt));
	
	if (keys[SDL_SCANCODE_KP_4])
		Get(currentObj).transform.AddYPR({speedRot * dt, 0.0f, 0.0f});
	if (keys[SDL_SCANCODE_KP_6])
		Get(currentObj).transform.AddYPR({-speedRot * dt, 0.0f, 0.0f});
	
	if (keys[SDL_SCANCODE_KP_PLUS])
		Get(currentObj).transform.AddScale(speedSca * dt);
	if (keys[SDL_SCANCODE_KP_MINUS])
		Get(currentObj).transform.AddScale(-speedSca * dt);
}


#endif
