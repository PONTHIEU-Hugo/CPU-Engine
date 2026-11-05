#ifndef ANIMATOR_SCENE_CPP_INCLUDED
#define ANIMATOR_SCENE_CPP_INCLUDED

#include "AnimatorScene.h"
#include "../GeometryFactory.hpp"

#include "../Shaders.h"

void AnimatorScene::Init()
{
    m_mesh = m_model.Load("Assets/dance.fbx"); 
    m_anim = new Animation("Assets/dance.fbx", &m_model);
    m_animator = new Animator(m_anim);
    
    m_groundMesh = GeometryFactory::BuildPlane();
    Add(m_groundMesh, m_groundMaterial, true,
        new Transform({0.0f, -2.0f, 0.0f}, {1,0,0,0}, glm::vec3(10.0f)));
    
    m_texture.Load("Assets/Character_Male_34.png");
    m_material.shader = &ShadowLambertShader;
    
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

void AnimatorScene::LoadSkybox()
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

void AnimatorScene::Update(float _dt)
{
    Get(m_skybox).transform->SetPosition(camera->GetPosition());
    camera->HandleEvent(_dt);
    Scene::Update(_dt);
}

void AnimatorScene::HandleEvent(float _dt)
{
    
}
#endif
