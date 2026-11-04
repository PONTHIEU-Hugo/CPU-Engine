#ifndef RENDU_SCENE_CPP_INCLUDED
#define RENDU_SCENE_CPP_INCLUDED

#include "RenduScene.h"
#include "../GeometryFactory.hpp"
#include "../Shaders.h"

void RenduScene::Init()
{
    m_roof              = GeometryFactory::LoadGeometry("Assets/Maison/roof.obj");
    m_wallOfRock        = GeometryFactory::LoadGeometry("Assets/Maison/wall.obj");
    m_wallOfConcrete    = GeometryFactory::LoadGeometry("Assets/Maison/concrete.obj");
    m_wood              = GeometryFactory::LoadGeometry("Assets/Maison/wood.obj");
    m_assets            = GeometryFactory::LoadGeometry("Assets/Maison/asset.obj");
    
    Add(m_roof, m_roofMaterial, true,
         new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_wallOfRock, m_rockMaterial, true,
         new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_wallOfConcrete, m_concreteMaterial, true,
        new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_wood, m_woodMaterial, true,
         new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_assets, m_assetMaterial, true,
         new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    
    
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
    m_displacementWood.Load("Assets/maple/displacement.png");
    
    m_assetTexture.Load("Assets/Maison/assets.png");
    
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
    m_woodMaterial.displacementMap          = &m_displacementWood;
    m_woodMaterial.displacementScale        = 0.05f;
    m_woodMaterial.normalFlipY              = true;
    
    m_assetMaterial.shader                   = &ShadowLambertShader;
    m_assetMaterial.texture                  = &m_assetTexture;
    
    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera({0.0f, 0.0f, 1.0f}, nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    m_transforms.push_back(&camera->transform);

    sun.direction           = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.ambient             = 0.8f;
    sun.shadowRadius        = 50.0f;
    sun.shadowCasterExtra   = 30.0f;
    sun.color            = {255.0f, 255.0f, 255.0f};
    sun.shadowFocus         = camera->GetPosition();
    
    LoadSkybox();
}

void RenduScene::Update(float _dt)
{
    camera->HandleEvent(_dt);
    glm::vec3 cam = camera->GetPosition();
    sun.shadowFocus = glm::vec3(cam.x, 0, cam.z);
    HandleEvent(_dt);
    Scene::Update(_dt);
}

void RenduScene::LoadSkybox()
{
    
}

void RenduScene::HandleEvent(float _dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
	
    if (keys[SDL_SCANCODE_R])
    {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
        sun.direction = glm::vec3(rot * glm::vec4(sun.direction, 0.0f));
    }
}

#endif
