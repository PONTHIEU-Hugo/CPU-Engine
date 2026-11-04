#ifndef RENDU_SCENE_CPP_INCLUDED
#define RENDU_SCENE_CPP_INCLUDED

#include "RenduScene.h"
#include "../GeometryFactory.hpp"
#include "../Shaders.h"


void RenduScene::SetWeather(Weather w)
{
    sun.direction = glm::normalize(glm::vec3(-0.15f, -1.0f, -0.1f)); 

    auto& cloud = m_cloud->GetSettings();
    auto& snow  = m_snow->GetSettings();

    switch (w)
    {
    case Weather::Clear:
        sun.color   = { 255, 245, 230 };
        sun.ambient = 0.2f;
        clearColor    = { 120, 170, 230 };
        cloud.emissionRate = 8.0f;
        cloud.colorStart   = { 255, 255, 255, 90 };
        cloud.colorEnd     = { 235, 240, 250, 70 };
        snow.emissionRate  = 0.0f;
        break;

    case Weather::Cloudy:
        sun.color   = { 175, 180, 190 };
        sun.ambient = 0.5f;
        clearColor    = { 165, 172, 182 };
        cloud.emissionRate = 100.0f;
        cloud.colorStart   = { 150, 155, 165, 110 };
        cloud.colorEnd     = { 110, 115, 125, 100 };
        snow.emissionRate  = 0.0f;
        break;

    case Weather::Snowy:
        sun.color   = { 150, 158, 170 };
        sun.ambient = 0.6f;
        clearColor    = { 190, 196, 205 };
        cloud.emissionRate = 35.0f;
        cloud.colorStart   = { 185, 190, 200, 110 };
        cloud.colorEnd     = { 150, 155, 165, 100 };
        snow.emissionRate  = 2000.0f;
        break;
    }
}

void RenduScene::Init()
{
    m_ground              = GeometryFactory::BuildPlane();
    m_roof              = GeometryFactory::LoadGeometry("Assets/Maison/roof2.obj");
    m_wallOfRock        = GeometryFactory::LoadGeometry("Assets/Maison/wall2.obj");
    m_wallOfConcrete    = GeometryFactory::LoadGeometry("Assets/Maison/concrete2.obj");
    m_wood              = GeometryFactory::LoadGeometry("Assets/Maison/wood2.obj");
    m_assets            = GeometryFactory::LoadGeometry("Assets/Maison/asset2.obj");
    
    Add(m_ground, m_groundMaterial, true,
         new Transform(glm::vec3(0.0f, -10.0f, 0.0f), {1,0,0,0}, glm::vec3(256.0f)));
    
    glm::vec3 posHouse = glm::vec3(2.00f, -1.15f, 7.00f);
    glm::vec3 scaleHouse = glm::vec3(0.33f, 0.33f, 0.33f);
    m_roofID = Add(m_roof, m_roofMaterial, true,
         new Transform(posHouse, {1,0,0,0}, scaleHouse));
    m_wallOfRockID = Add(m_wallOfRock, m_rockMaterial, true,
         new Transform(posHouse, {1,0,0,0}, scaleHouse));
    m_wallOfConcreteID = Add(m_wallOfConcrete, m_concreteMaterial, true,
        new Transform(posHouse, {1,0,0,0}, scaleHouse));
    m_woodID = Add(m_wood, m_woodMaterial, true,
         new Transform(posHouse, {1,0,0,0}, scaleHouse));
    m_assetsID = Add(m_assets, m_assetMaterial, true,
         new Transform(posHouse, {1,0,0,0}, scaleHouse));
    
    
    m_roofTexture.Load("Assets/roof/albedo.png");
    m_normalMapRoof.Load("Assets/roof/normal.png");
    m_armRoof.Load("Assets/roof/arm.png");
    m_displacementRoof.Load("Assets/roof/displacement.png");
    
    m_rockTexture.Load("Assets/rock1k/albedo.png");
    m_normalMapRock.Load("Assets/rock1k/normal.png");
    m_armRock.Load("Assets/rock1k/arm.png");
    m_displacementRock.Load("Assets/rock1k/displacement.png");
    
    m_concreteTexture.Load("Assets/concrete/albedo.png");
    m_normalMapConcrete.Load("Assets/concrete/normal.png");
    m_armConcrete.Load("Assets/concrete/arm.png");
    m_displacementConcrete.Load("Assets/concrete/displacement.png");
    
    m_woodTexture.Load("Assets/maple/albedo.png");
    m_normalMapWood.Load("Assets/maple/normal.png");
    m_armWood.Load("Assets/maple/arm.png");
    
    m_assetTexture.Load("Assets/Maison/assets.png");
    
    m_groundMaterial.shader              = &ShadowLambertShader;
    m_groundMaterial.baseColor           = ToColor(0xC2B280);
    
    m_roofMaterial.shader                = &ShadowLambertShader;
    m_roofMaterial.texture               = &m_roofTexture;
    m_roofMaterial.normalMap             = &m_normalMapRoof;
    m_roofMaterial.armMap                = &m_armRoof;
    m_roofMaterial.displacementMap       = &m_displacementRoof;
    m_roofMaterial.displacementScale     = 0.1f;
    m_roofMaterial.normalFlipY           = true;
    
    m_rockMaterial.shader                   = &ShadowLambertShader;
    m_rockMaterial.texture                  = &m_rockTexture;
    m_rockMaterial.normalMap                = &m_normalMapRock;
    m_rockMaterial.armMap                   = &m_armRock;
    m_rockMaterial.displacementMap          = &m_displacementRock;
    m_rockMaterial.displacementScale        = 0.5f;
    m_rockMaterial.normalFlipY              = true;
    
    m_concreteMaterial.shader               = &ShadowLambertShader;
    m_concreteMaterial.texture              = &m_concreteTexture;
    m_concreteMaterial.normalMap            = &m_normalMapConcrete;
    m_concreteMaterial.armMap               = &m_armConcrete;
    m_concreteMaterial.displacementMap      = &m_displacementConcrete;
    m_concreteMaterial.displacementScale    = 0.01f;
    m_concreteMaterial.normalFlipY          = true;
    
    m_woodMaterial.shader                   = &ShadowLambertShader;
    m_woodMaterial.texture                  = &m_woodTexture;
    m_woodMaterial.normalMap                = &m_normalMapWood;
    m_woodMaterial.armMap                   = &m_armWood;
    m_woodMaterial.displacementScale        = 0.05f;
    m_woodMaterial.normalFlipY              = true;
    
    m_assetMaterial.shader                   = &ShadowLambertShader;
    m_assetMaterial.texture                  = &m_assetTexture;
    
    m_Terrain.coneWeight = 0.1;
    m_Terrain.sharpness = 1.0;
    m_Terrain.GenerateHeighmap(glm::vec2(50, 50), HeighMapType::_OCTAVE_PERLIN_2D, 0.25f, 0.2f, 1.0f, true);	
    ObjectId terrainID = Add(m_Terrain.GetMesh(), m_Terrain.GetMaterial());
    Transform* transformTerrain = Get(terrainID).transform;
    transformTerrain->SetPosition({ 0, 0, 0 });
    float scale = 10.0f;
    transformTerrain->SetScale(glm::vec3(scale, scale, scale));
    
    m_ocean.GenerateMesh({ 256, 256 }, 1.5f);                 
    m_ocean.AddWave({ 1.0f,  0.3f }, 0.12f, 0.8f, 60.0f);      
    m_ocean.AddWave({ 1.0f, -0.2f }, 0.07f, 0.7f, 35.0f);
    m_ocean.AddWave({ 0.7f,  0.7f }, 0.035f, 0.6f, 20.0f);
    m_ocean.AddWave({ 1.0f,  0.8f }, 0.015f, 0.5f, 12.0f);      

    m_oceanID = Add(m_ocean.m_Mesh, m_ocean.m_Material, false);
    m_oceanTransform = Get(m_oceanID).transform;
    m_oceanTransform->SetPosition({ 0, -6.0f, 0 });
    
    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera(glm::vec3(30.0f, 30.0f, 30.0f), nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.5f, 1000.0f);
    camera->transform.LookAt(transformTerrain->GetPosition());
    m_transforms.push_back(&camera->transform);

    sun.direction           = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.ambient             = 0.1f;
    sun.shadowRadius        = 500.0f;
    sun.shadowCasterExtra   = 300.0f;
    sun.color            = {247, 202, 74};
    sun.shadowFocus         = camera->GetPosition();
    
    m_snow = new SnowEmitter(10000000);
    m_snow->swayFrequency = 5.0f;
    m_snow->Init();
    m_snow->GetSettings().radius = m_Terrain.GetMesh().localBounds.max.x * scale * 2.0f;
    
    AddEmitter(*m_snow);
    
    m_cloud = new CloudEmitter(50000);
    m_cloud->swayAmplitude = 0.15f;
    m_cloud->swayFrequency = 0.3f;
    m_cloud->Init();
    m_cloud->SetOrigin({ 0.0f, 5.0f, 0.0f });
    AddEmitter(*m_cloud);
    
    SetWeather(Weather::Clear);
    LoadSkybox();
}

void RenduScene::Update(float _dt)
{
    camera->HandleEvent(_dt);
    glm::vec3 cam = camera->GetPosition();
    sun.shadowFocus = glm::vec3(cam.x, 0, cam.z);
    HandleEvent(_dt);
    
    m_snow->SetOrigin(glm::vec3(0.0f, 20.0f, 0.0f));
    m_snow->Update(_dt);
    m_cloud->Update(_dt);
    
    Scene::Update(_dt);
}

void RenduScene::LoadSkybox()
{
    
}

void RenduScene::Move(glm::vec3 dir)
{
    Get(m_roofID).transform->Move(dir);
    Get(m_wallOfRockID).transform->Move(dir);
    Get(m_wallOfConcreteID).transform->Move(dir);
    Get(m_woodID).transform->Move(dir);
    Get(m_assetsID).transform->Move(dir);
}

void RenduScene::HandleEvent(float _dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
	
    if (keys[SDL_SCANCODE_R])
    {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
        sun.direction = glm::vec3(rot * glm::vec4(sun.direction, 0.0f));
    }
    
    if (keys[SDL_SCANCODE_1]) SetWeather(Weather::Clear);
    if (keys[SDL_SCANCODE_2]) SetWeather(Weather::Cloudy);
    if (keys[SDL_SCANCODE_3]) SetWeather(Weather::Snowy);
}

#endif
