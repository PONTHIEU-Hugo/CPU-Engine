#ifndef MATERIAL_SCENE_CPP_INCLUDED
#define MATERIAL_SCENE_CPP_INCLUDED

#include "MaterialScene.h"
#include "../GeometryFactory.hpp"
#include "../Shaders.h"

void MaterialScene::Init()
{
    m_sphereMesh = GeometryFactory::BuildIcosphere(5);
    m_groundMesh = GeometryFactory::LoadGeometry("Assets/de_dust2.obj");
    
    Add(m_sphereMesh, m_defaultMaterial, true,
        new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_groundMesh, m_groundMaterial, true,
        new Transform({0.0f, -2.0f, 0.0f}, {1,0,0,0}, glm::vec3(10.0f)));
    
    m_defaultMaterial.shader = &RefractionShader;
    m_defaultMaterial.baseColor = glm::vec4(255.0f, 255.0f, 255.0f, 240.0f);
    m_defaultMaterial.transparent = true;
    m_defaultMaterial.thickness   = 1.0f;
    m_defaultMaterial.eta         = 0.8f;
    
    m_groundTexture.Load("Assets/de_dust2.png");
    m_groundMaterial.shader = &LambertShader;
    m_groundMaterial.texture = &m_groundTexture;
    
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
