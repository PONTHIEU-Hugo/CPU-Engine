#ifndef MAIN_SCENE_CPP_INCLUDED
#define MAIN_SCENE_CPP_INCLUDED

#include "MainScene.h"

#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../Shaders.h"
#include "../GeometryFactory.hpp"

void MainScene::Init()
{
	m_carMesh = GeometryFactory::LoadGeometry("Assets/car.obj");
	m_wheelArMesh = GeometryFactory::LoadGeometry("Assets/wheelAr.obj");
	m_wheelAvMesh = GeometryFactory::LoadGeometry("Assets/wheelAv.obj");
	
	m_carTexture.Load("Assets/vivien.png");

	// Materiaux
	m_defaultMaterial.shader = &ShadowLambertShader;

	m_carMaterial = m_defaultMaterial;
	m_carMaterial.baseColor.a = 255;
	m_carMaterial.transparent = false;
	if (m_carTexture.IsValid()) 
		m_carMaterial.texture = &m_carTexture;

	m_defaultMaterial.specular  = 0.8f;  
	m_defaultMaterial.roughness = 0.3f;

	m_carMaterial.specular      = 0.5f;
	m_carMaterial.roughness     = 0.4f;

	// Scene 
	m_car = Add(m_carMesh, m_carMaterial, true,
		new Transform({-10.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	m_wheelAr = Add(m_wheelArMesh, m_carMaterial, true,
			new Transform({0.0f, 1.1f, 5.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	m_wheelAv = Add(m_wheelAvMesh, m_carMaterial, true,
	 		new Transform({0.0f, 1.05f, -5.5f}, {1,0,0,0}, glm::vec3(1.0f)));
	
	Transform* carT = Get(m_car).transform;
	Get(m_wheelAr).transform->SetParent(carT);
	Get(m_wheelAv).transform->SetParent(carT);
	
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
	Add(m_Terrain.GetMesh(), m_Terrain.GetMaterial(), true,
		new Transform({0.0f, -1.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
	
	m_Ocean.GenerateMesh({ 250, 250 });
	m_Ocean.AddWave({ 1.0f,  0.3f }, 2.0f, 0.8f, 40.0f);  
	m_Ocean.AddWave({ 1.0f, -0.2f }, 1.2f, 0.7f, 25.0f);   
	m_Ocean.AddWave({ 0.7f,  0.7f }, 0.6f, 0.6f, 15.0f);  
	m_Ocean.AddWave({ 1.0f,  0.8f }, 0.3f, 0.5f, 9.0f);
	m_OceanID = Add(m_Ocean.m_Mesh, m_Ocean.m_Material, false);
	RenderObject& ocean = Get(m_OceanID);
	ocean.transform->SetPosition({ 0, -1.0f, 12.5f});
	ocean.transform->SetScale(glm::vec3{ 0.5f, 0.5f, 0.5f } * 0.1f);
	
	// Systèmes
	LoadSkybox();
	
	m_snow.groundHeight = 20.0f;

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

	Get(m_skybox).transform->SetPosition(camera->GetPosition());

	const glm::vec3 exhaustLocalOffset(1.7f, 1.3f, 9.2f);
	const glm::mat4& carWorld = Get(m_car).transform->GetWorldMatrix();
	m_smoke.SetOrigin(glm::vec3(carWorld * glm::vec4(exhaustLocalOffset, 1.0f)));
	m_smoke.Update(_dt);

	const glm::vec3 roofLocalOffset(0.0f, 10.0f, 0.0f);
	m_sparkle.SetOrigin(glm::vec3(carWorld * glm::vec4(roofLocalOffset, 1.0f)));
	m_sparkle.Update(_dt);

	glm::vec3 cam = camera->GetPosition();
	m_snow.SetOrigin(cam + glm::vec3(0.0f, 20.0f, 0.0f));
	m_snow.Update(_dt);

	sun.shadowFocus = glm::vec3(cam.x, 0.0f, cam.z);
	Scene::Update(_dt);
}

void MainScene::LoadSkybox()
{
	m_skyboxMesh = GeometryFactory::BuildSkybox();

	cubemap.faces[0].Load("Assets/right.jpg");
	cubemap.faces[1].Load("Assets/left.jpg");
	cubemap.faces[2].Load("Assets/top.jpg");
	cubemap.faces[3].Load("Assets/bottom.jpg");
	cubemap.faces[4].Load("Assets/front.jpg");
	cubemap.faces[5].Load("Assets/back.jpg");

	m_skyboxMaterial.cubeMap = &cubemap;
	m_skyboxMaterial.cull = CullMode::None;
	m_skyboxMaterial.isSkybox = true;
	m_skyboxMaterial.shader = &SkyboxShader;

	m_skybox = Add(m_skyboxMesh, m_skyboxMaterial, false);
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
	if (bDown && m_pWasDown == false)
		m_sparkle.Burst(200);
	m_pWasDown = bDown;
	
	if (keys[SDL_SCANCODE_R])
	{
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
		sun.direction = glm::vec3(rot * glm::vec4(sun.direction, 0.0f));
	}
	
	float dt = _dt;
	float speed = 20.0f;
	float speedRot = 10.0f;
	float speedSca = 0.2f;
	
	ObjectId currentObj = m_OceanID;
	
	if (keys[SDL_SCANCODE_KP_MULTIPLY])
	{
		m_OceanAmplitude++;
		//m_Ocean.SetAmplitude(m_OceanAmplitude);
	}
	else if(keys[SDL_SCANCODE_KP_DIVIDE])
	{
		m_OceanAmplitude--;
		//m_Ocean.SetAmplitude(m_OceanAmplitude);
	}

	if (keys[SDL_SCANCODE_KP_8])
	{
		Get(currentObj).transform->Move(Get(currentObj).transform->GetForward() * (speed * dt));
		if ( m_wheelAr != -1) Get(m_wheelAr).transform->AddYPR({0.0f, -speedRot * dt, 0.0f});
		if ( m_wheelAv != -1) Get(m_wheelAv).transform->AddYPR({0.0f, -speedRot * dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_5])
	{
		Get(currentObj).transform->Move(Get(currentObj).transform->GetForward() * (-speed * dt));
		if ( m_wheelAr != -1) Get(m_wheelAr).transform->AddYPR({0.0f, speedRot * dt, 0.0f});
		if ( m_wheelAv != -1) Get(m_wheelAv).transform->AddYPR({0.0f, speedRot * dt, 0.0f});
	}
	if (keys[SDL_SCANCODE_KP_1])
		Get(currentObj).transform->Move(Get(currentObj).transform->GetUp() * (-speed * dt));
	if (keys[SDL_SCANCODE_KP_3])
		Get(currentObj).transform->Move(Get(currentObj).transform->GetUp() * (speed * dt));
	
	if (keys[SDL_SCANCODE_KP_4])
		Get(currentObj).transform->AddYPR({speedRot * dt, 0.0f, 0.0f});
	if (keys[SDL_SCANCODE_KP_6])
		Get(currentObj).transform->AddYPR({-speedRot * dt, 0.0f, 0.0f});
	
	if (keys[SDL_SCANCODE_KP_PLUS])
		Get(currentObj).transform->AddScale(speedSca * dt);
	if (keys[SDL_SCANCODE_KP_MINUS])
		Get(currentObj).transform->AddScale(-speedSca * dt);
}


#endif
