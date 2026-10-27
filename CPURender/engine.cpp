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

	m_Terrain.GenerateHeighmap(glm::vec2(50, 50), HeighMapType::_PERLIN_2D);
	m_Terrain.SetMaterial(&m_defaultMaterial);
	m_scene.Add(m_Terrain.GetMesh(), m_Terrain.GetMaterial());
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

	m_emitter.Update(dt);

	// La shadow map suit la camera, au niveau du sol
	glm::vec3 cam = m_scene.camera.GetCameraPos();
	m_scene.sun.shadowFocus = glm::vec3(cam.x, m_groundMesh.localBounds.min.y, cam.z);
}

void Engine::handleEvent(const SDL_Event& event)
{
	// event est une union : ne lire event.key que pour un evenement clavier
	if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
		return;

	if (event.key.key == SDLK_R)
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
		m_scene.sun.direction = glm::vec3(rot * glm::vec4(m_scene.sun.direction, 0.0f));
	}
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	m_renderer.Render(m_scene, frameBufferOut);
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
