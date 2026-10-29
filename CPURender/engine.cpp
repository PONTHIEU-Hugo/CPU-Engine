#include "engine.h"

#include <cstdio>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Chrono.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"

static Mesh BuildGround(float y, float half)
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

	Mesh mesh;
	mesh.vertices = { p0, p1, p2, p0, p2, p3 };
	mesh.ComputeBounds();
	return mesh;
}

void Engine::init()
{
	printf("Init engine \n");

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
	
	// Ressources
	m_cubeMesh = GeometryFactory::BuildUVSphere(32, 16, 1.0f);
	m_carMesh = GeometryFactory::LoadGeometry("Assets/car.obj");
	m_wheelArMesh = GeometryFactory::LoadGeometry("Assets/wheelAr.obj");
	m_wheelAvMesh = GeometryFactory::LoadGeometry("Assets/wheelAv.obj");
	
	m_carTexture.Load("Assets/vivien.png");

	const AABB& carBounds = m_carMesh.localBounds;
	float carRadius = glm::length(carBounds.max - carBounds.min) * 0.5f;
	m_groundMesh = BuildGround(carBounds.min.y, carRadius * 5.0f);

	// Materiaux
	m_defaultMaterial.shader = &ShadowLambertShader;

	m_carMaterial = m_defaultMaterial;
	if (m_carTexture.IsValid()) 
		m_carMaterial.texture = &m_carTexture;

	m_groundMaterial      = m_defaultMaterial;
	m_groundMaterial.cull = CullMode::None;

	m_defaultMaterial.specular  = 0.8f;  
	m_defaultMaterial.roughness = 0.3f;

	m_carMaterial.specular      = 0.5f;
	m_carMaterial.roughness     = 0.4f;

	// Scene 
	//ObjectId cube = m_scene.Add(m_cubeMesh, m_defaultMaterial);
	//m_scene.Get(cube).transform.SetPosition({ 0.0f, 0.5f, 0.0f });

	m_car = m_scene.Add(m_carMesh, m_carMaterial, true,
		Transform({-10.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	ObjectId wheelAr = m_scene.Add(m_wheelArMesh, m_carMaterial, true,
			Transform({0.0f, 1.1f, 5.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	ObjectId wheelAv = m_scene.Add(m_wheelAvMesh, m_carMaterial, true,
			Transform({0.0f, 1.05f, -5.5f}, {1,0,0,0}, glm::vec3(1.0f)));
	m_scene.Add(m_cubeMesh, m_defaultMaterial, true,
			Transform({0.0f, 1.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	m_scene.Add(m_groundMesh, m_groundMaterial, false);

	Transform& carT = m_scene.Get(m_car).transform;
	m_scene.Get(wheelAr).transform.SetParent(&carT);
	m_scene.Get(wheelAv).transform.SetParent(&carT);
	
	float aspect = float(WIDTH) / float(HEIGHT);
	m_scene.camera = new Camera({0.0f, 4.5f, 3.0f}, &carT);
	// m_scene.camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_scene.camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	m_transforms.push_back(&m_scene.camera->transform);

	m_scene.sun.direction         = glm::vec3(-0.5f, -1.0f, -0.3f);
	m_scene.sun.ambient           = 0.2f;
	m_scene.sun.shadowRadius      = 50.0f;
	m_scene.sun.shadowCasterExtra = 30.0f;
	m_scene.sun.shadowFocus       = m_scene.camera->GetPosition();

	m_Terrain.GenerateHeighmap(glm::vec2(50, 50), HeighMapType::_PERLIN_2D);
	m_Terrain.SetMaterial(&m_defaultMaterial);
	//m_scene.Add(m_Terrain.GetMesh(), m_Terrain.GetMaterial());

	// Systèmes
	m_scene.LoadSkybox();
	m_renderer.Init(512);
	m_renderer.showShadowDebug = false;

	// Feu : monte avec une legere dispersion, de blanc a rouge puis sombre (palette du feu).
	m_emitter.settings.emissionRate = 250.0f;
	m_emitter.settings.lifeMin      = 0.6f;
	m_emitter.settings.lifeMax      = 1.2f;
	m_emitter.settings.speedMin     = 2.0f;
	m_emitter.settings.speedMax     = 5.0f;
	m_emitter.settings.coneAngle    = 25.0f;
	m_emitter.settings.acceleration = glm::vec3(0.0f, 6.0f, 0.0f);
	m_emitter.settings.sizeStart    = 0.35f;
	m_emitter.settings.sizeEnd      = 0.05f;
	m_emitter.Init();
	m_scene.AddEmitter(m_emitter);
}

void Engine::Update()
{
	const float dt = static_cast<float>(CHRONO->GetDeltaTime());

	m_scene.camera->HandleEvent(dt);

	for (Transform* obj : m_transforms)
		obj->Update();

	// L'emetteur suit l'echappement de la voiture (decalage local, a ajuster selon le modele).
	const glm::vec3 exhaustLocalOffset(1.7f, 1.3f, 9.2f);
	const glm::mat4& carWorld = m_scene.Get(m_car).transform.GetWorldMatrix();
	m_emitter.SetOrigin(glm::vec3(carWorld * glm::vec4(exhaustLocalOffset, 1.0f)));
	m_emitter.Update(dt);

	glm::vec3 cam = m_scene.camera->GetPosition();
	m_scene.sun.shadowFocus = glm::vec3(cam.x, m_groundMesh.localBounds.min.y, cam.z);
}

void Engine::handleEvent()
{
	const bool* keys = SDL_GetKeyboardState(nullptr);

	// P : active / desactive l'emission (une seule bascule par appui)
	bool pDown = keys[SDL_SCANCODE_P];
	if (pDown && m_pWasDown == false)
		m_emitter.SetActive(m_emitter.IsActive() == false);
	m_pWasDown = pDown;

	if (keys[SDL_SCANCODE_R])
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
		m_scene.sun.direction = glm::vec3(rot * glm::vec4(m_scene.sun.direction, 0.0f));
	}
	
	float dt = CHRONO->GetDeltaTime();
	float speed = 20.0f;
	float speedRot = 10.0f;
	float speedSca = 0.2f;
	
	ObjectId currentObj = m_car;
	
	if (keys[SDL_SCANCODE_KP_8])
	{
		m_scene.Get(currentObj).transform.Move(m_scene.Get(currentObj).transform.GetForward() * (speed * dt));
		m_scene.Get(2).transform.AddYPR({0.0f, -speedRot * dt, 0.0f});
		m_scene.Get(3).transform.AddYPR({0.0f, -speedRot * dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_5])
	{
		m_scene.Get(currentObj).transform.Move(m_scene.Get(currentObj).transform.GetForward() * (-speed * dt));
		m_scene.Get(2).transform.AddYPR({0.0f, speedRot * 0.5 * dt, 0.0f});
		m_scene.Get(3).transform.AddYPR({0.0f, speedRot * 0.5* dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_1])
		m_scene.Get(currentObj).transform.Move(m_scene.Get(currentObj).transform.GetUp() * (-speed * dt));
	if (keys[SDL_SCANCODE_KP_3])
		m_scene.Get(currentObj).transform.Move(m_scene.Get(currentObj).transform.GetUp() * (speed * dt));
	
	if (keys[SDL_SCANCODE_KP_4])
		m_scene.Get(currentObj).transform.AddYPR({speedRot * dt, 0.0f, 0.0f});
	if (keys[SDL_SCANCODE_KP_6])
		m_scene.Get(currentObj).transform.AddYPR({-speedRot * dt, 0.0f, 0.0f});
	
	if (keys[SDL_SCANCODE_KP_PLUS])
		m_scene.Get(currentObj).transform.AddScale(speedSca * dt);
	if (keys[SDL_SCANCODE_KP_MINUS])
		m_scene.Get(currentObj).transform.AddScale(-speedSca * dt);
	
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	m_renderer.Render(m_scene, frameBufferOut);
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
