#ifndef MATERIAL_SCENE_CPP_INCLUDED
#define MATERIAL_SCENE_CPP_INCLUDED

#include "MaterialScene.h"
#include "../GeometryFactory.hpp"
#include "../Shaders.h"

void MaterialScene::Init()
{
    m_sphereMesh    = GeometryFactory::BuildCube();
    m_cubeMesh      = GeometryFactory::BuildCube();
    
    Add(m_sphereMesh, m_defaultMaterial, true,
        new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_cubeMesh, m_cubeMaterial, true,
        new Transform({-2.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    
    
    m_cubeTexture.Load("Assets/rock/rock_albedo.jpg");
    m_cubeNormalMap.Load("Assets/rock/rock_normal.png");
    m_cubeMaterial.shader = &LambertShader;
    m_cubeMaterial.texture = &m_cubeTexture;
    
    m_defaultMaterial = m_cubeMaterial;
    
    m_cubeMaterial.normalMap = &m_cubeNormalMap;
    m_cubeMaterial.normalFlipY = true;
    
    
    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera({0.0f, 0.0f, 3.0f}, nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    m_transforms.push_back(&camera->transform);

    sun.direction         = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.ambient           = 0.2f;
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
    skybox.faces[0].Load("Assets/skybox/de_dust2/right.png");
    skybox.faces[1].Load("Assets/skybox/de_dust2/left.png");
    skybox.faces[2].Load("Assets/skybox/de_dust2/top.png");
    skybox.faces[3].Load("Assets/skybox/de_dust2/bottom.png");
    skybox.faces[4].Load("Assets/skybox/de_dust2/front.png");
    skybox.faces[5].Load("Assets/skybox/de_dust2/back.png");
}

void MaterialScene::HandleEvent(float _dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
	
    if (keys[SDL_SCANCODE_R])
    {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0, 1, 0));
        sun.direction = glm::vec3(rot * glm::vec4(sun.direction, 0.0f));
    }
    
    if (keys[SDL_SCANCODE_RETURN])
        EnableSkybox(!skybox.enabled);
}

#endif
