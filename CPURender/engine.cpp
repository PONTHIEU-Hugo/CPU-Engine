#include "engine.h"

#include <cstdio>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Chrono.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"

// Quad horizontal qui reçoit les ombres
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
	
	m_transforms.push_back(&m_car.transform);
	m_transforms.push_back(&m_wheelAv.transform);
	m_transforms.push_back(&m_wheelAr.transform);
	m_transforms.push_back(&m_cube.transform);
	m_transforms.push_back(&m_triangle.transform);
	m_transforms.push_back(&m_ground.transform);
	m_transforms.push_back(&m_camera->transform);

	m_renderer.Init(512);
	m_renderer.showShadowDebug = false;
	m_emitter.Init();
	// Ressources
	m_cubeMesh = GeometryFactory::BuildUVSphere(32, 16, 1.0f);

	m_carMesh = GeometryFactory::LoadGeometry("Assets/tuture.obj");
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

	m_car = m_scene.Add(m_carMesh, m_carMaterial);

	m_scene.Add(m_groundMesh, m_groundMaterial, false);

	float aspect = float(WIDTH) / float(HEIGHT);
	// m_scene.camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_scene.camera.SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

	m_scene.sun.direction         = glm::vec3(-0.5f, -1.0f, -0.3f);
	m_scene.sun.ambient           = 0.2f;
	m_scene.sun.shadowRadius      = 50.0f;
	m_scene.sun.shadowCasterExtra = 30.0f;
	m_scene.sun.shadowFocus       = m_scene.camera.GetCameraPos();

	// Systèmes
	m_renderer.Init(512);
	m_renderer.showShadowDebug = false;
	m_emitter.Init();
}

void Engine::Update()
{
	// dt lu une seule fois, puis passe aux systemes
	const float dt = static_cast<float>(CHRONO->GetDeltaTime());

	m_scene.camera.HandleEvent(dt);

	// Deplacement de la voiture : en continu (etat clavier), independant de la repetition des touches
	const bool* keys = SDL_GetKeyboardState(nullptr);
	const float carSpeed = 8.0f;
	glm::vec3 move(0.0f);
	if (keys[SDL_SCANCODE_KP_8]) move.z += 1.0f;
	if (keys[SDL_SCANCODE_KP_5]) move.z -= 1.0f;
	if (keys[SDL_SCANCODE_KP_4]) move.x -= 1.0f;
	if (keys[SDL_SCANCODE_KP_6]) move.x += 1.0f;
	if (move != glm::vec3(0.0f))
		m_scene.Get(m_car).transform.Move(move * carSpeed * dt);

	for (Transform* obj : m_transforms)
		obj->Update();

	m_emitter.Update(dt);

	// La shadow map suit la camera, au niveau du sol
	glm::vec3 cam = m_scene.camera.GetCameraPos();
	m_scene.sun.shadowFocus = glm::vec3(cam.x, m_groundMesh.localBounds.min.y, cam.z);
}

void Engine::handleEvent()
{
	const bool* keys = SDL_GetKeyboardState(nullptr);
	
	if (keys[SDL_SCANCODE_R])
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
		m_scene.sun.direction = glm::vec3(rot * glm::vec4(m_scene.sun.direction, 0.0f));
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
	m_renderer.Render(m_scene, frameBufferOut);
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
