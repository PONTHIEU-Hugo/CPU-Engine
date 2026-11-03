#ifndef ANIMATOR_SCENE_CPP_INCLUDED
#define ANIMATOR_SCENE_CPP_INCLUDED

#include "AnimatorScene.h"
#include "../GeometryFactory.hpp"

#include "../Shaders.h"

void AnimatorScene::Init()
{
    m_mesh = m_model.Load("Assets/dance2.dae"); 
    m_anim = new Animation("Assets/dance2.dae", &m_model);
    m_animator = new Animator(m_anim);

    m_groundMesh = GeometryFactory::BuildPlane();
    Add(m_groundMesh, m_groundMaterial, true,
            new Transform({0.0f, -2.0f, 0.0f}, {1,0,0,0}, glm::vec3(10.0f)));
    
    m_texture.Load("Assets/Character_Male_34.png");
    m_material.shader = &ShadowLambertShader;
    m_material.baseColor = glm::vec4(255.0f, 255.0f, 255.0f, 255.0f);
    
    m_groundMaterial = m_material;
    m_material.texture = &m_texture;

    ObjectId id = Add(m_mesh, m_material, true,
        new Transform({0.0f, -1.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Get(id).animator = m_animator; 
    
    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera({0.0f, 0.0f, 5.0f}, nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    m_transforms.push_back(&camera->transform);
    
    sun.direction         = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.ambient           = 0.2f;
    sun.shadowRadius      = 50.0f;
    sun.shadowCasterExtra = 30.0f;
    sun.shadowFocus       = camera->GetPosition();
    
    LoadSkybox();
    
}

void AnimatorScene::Update(float _dt)
{
    Scene::Update(_dt);
    camera->HandleEvent(_dt);
}

void AnimatorScene::LoadSkybox()
{
    skybox.faces[0].Load("Assets/right.jpg");
    skybox.faces[1].Load("Assets/left.jpg");
    skybox.faces[2].Load("Assets/top.jpg");
    skybox.faces[3].Load("Assets/bottom.jpg");
    skybox.faces[4].Load("Assets/front.jpg");
    skybox.faces[5].Load("Assets/back.jpg");
}

void AnimatorScene::HandleEvent(float _dt)
{
    
}
#endif
