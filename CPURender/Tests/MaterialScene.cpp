#ifndef MATERIAL_SCENE_CPP_INCLUDED
#define MATERIAL_SCENE_CPP_INCLUDED

#include "MaterialScene.h"
#include "../GeometryFactory.hpp"
#include "../Shaders.h"

void MaterialScene::Init()
{
    m_sphereMesh    = GeometryFactory::BuildUVSphere(16, 16);
    m_sphereIcoMesh    = GeometryFactory::BuildIcosphere(5);
    m_cubeMesh      = GeometryFactory::BuildCube();
    
     Add(m_sphereMesh, m_defaultMaterial, true,
         new Transform({2.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_sphereIcoMesh, m_sheetMaterial, true,
        new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
     Add(m_cubeMesh, m_defaultMaterial, true,
         new Transform({-2.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    
    
    m_defaultMaterial.shader = &ShadowLambertShader;
    m_texture.Load("Assets/rock/albedo.png");
    m_normalMap.Load("Assets/rock/normal.png");
    m_arm.Load("Assets/rock/arm.png");
    m_ao.Load("Assets/rock/ao.png");
    m_metalness.Load("Assets/rock/metalness.png");
    m_roughness.Load("Assets/rock/roughness.png");
    
    m_defaultMaterial.texture      = &m_texture;
    
    m_sheetMaterial                = m_defaultMaterial;
    m_sheetMaterial.armMap         = &m_arm;
    m_sheetMaterial.specular       = 1.0f;
    
    m_sheetMaterial.normalMap    = &m_normalMap;
    m_sheetMaterial.normalFlipY  = true;
    
    
    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera({0.0f, 0.0f, 1.0f}, nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    m_transforms.push_back(&camera->transform);

    sun.direction         = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.ambient           = 0.1f;
    sun.shadowRadius      = 50.0f;
    sun.shadowCasterExtra = 30.0f;
    sun.shadowFocus       = camera->GetPosition();
    
    LoadSkybox();
}

void MaterialScene::Update(float _dt)
{
    camera->HandleEvent(_dt);
    glm::vec3 cam = camera->GetPosition();
    sun.shadowFocus = glm::vec3(cam.x, 0, cam.z);
    HandleEvent(_dt);
    Scene::Update(_dt);
}

void MaterialScene::LoadSkybox()
{
}

void MaterialScene::HandleEvent(float _dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
	
    if (keys[SDL_SCANCODE_R])
    {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
        sun.direction = glm::vec3(rot * glm::vec4(sun.direction, 0.0f));
    }
}

#endif
