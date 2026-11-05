#ifndef MATERIAL_SCENE_CPP_INCLUDED
#define MATERIAL_SCENE_CPP_INCLUDED

#include "MaterialScene.h"
#include "../GeometryFactory.hpp"
#include "../Shaders.h"

void MaterialScene::Init()
{
    m_planeMesh     = GeometryFactory::BuildPlane();
    m_sphereMesh    = GeometryFactory::BuildIcosphere(5);
    m_sphereIcoMesh = GeometryFactory::BuildCube();
    m_sphereRefractionMesh      = GeometryFactory::BuildIcosphere(5);
    
    Add(m_planeMesh, m_defaultMaterial, true,
         new Transform({0.0f, -2.0f, 0.0f}, {1,0,0,0}, glm::vec3(10.0f)));
    Add(m_sphereMesh, m_rockMaterial, true,
         new Transform({2.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_sphereIcoMesh, m_sheetMaterial, true,
        new Transform({0.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    Add(m_sphereRefractionMesh, m_refractMaterial, true,
         new Transform({-2.0f, 0.0f, 0.0f}, {1,0,0,0}, glm::vec3(1.0f)));
    
    m_texture.Load("Assets/rock1k/albedo.png");
    m_normalMap.Load("Assets/rock1k/normal.png");
    m_arm.Load("Assets/rock1k/arm.png");
    m_displacement.Load("Assets/rock1k/displacement.png");
    
    m_texture4.Load("Assets/grass/albedo.png");
    m_normalMap4.Load("Assets/grass/normal.png");
    m_arm4.Load("Assets/grass/arm.png");
    m_displacement4.Load("Assets/grass/displacement.png");
    
    m_texture2.Load("Assets/sheet/albedo.png");
    m_normalMap2.Load("Assets/sheet/normal.png");
    m_arm2.Load("Assets/sheet/arm.png");
    m_displacement2.Load("Assets/sheet/displacement.png");
    
    
    m_defaultMaterial.shader                = &ShadowLambertShader;
    m_defaultMaterial.texture               = &m_texture4;
    m_defaultMaterial.normalMap             = &m_normalMap4;
    m_defaultMaterial.normalFlipY           = true;
    m_defaultMaterial.armMap                = &m_arm4;
    m_defaultMaterial.displacementMap       = &m_displacement4;
    m_defaultMaterial.displacementScale     = 0.1f;
    
    m_rockMaterial.shader                   = &ShadowLambertShader;
    m_rockMaterial.texture                  = &m_texture;
    m_rockMaterial.normalMap                = &m_normalMap;
    m_rockMaterial.armMap                   = &m_arm;
    m_rockMaterial.displacementMap          = &m_displacement;
    m_rockMaterial.displacementScale        = 0.5f;
    m_rockMaterial.normalFlipY              = true;
    
    m_sheetMaterial.shader                  = &ShadowLambertShader;
    m_sheetMaterial.texture                 = &m_texture2;
    m_sheetMaterial.normalMap               = &m_normalMap2;
    m_sheetMaterial.armMap                  = &m_arm2;
    m_sheetMaterial.displacementMap         = &m_displacement2;
    m_sheetMaterial.displacementScale       = 0.01f;
    m_sheetMaterial.displacementPixelEdge   = 4.0f;
    m_sheetMaterial.normalFlipY             = true;
    
    m_refractMaterial.shader                   = &RefractionShader;
    m_refractMaterial.specular                 = 1.0f;
    m_refractMaterial.eta                       = 0.66f;
    m_refractMaterial.thickness                 = 0.75f;
    m_refractMaterial.transparent               = true;
    m_refractMaterial.baseColor.a               = 128.0f;
    
    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera({0.0f, 0.0f, 1.0f}, nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    m_transforms.push_back(&camera->transform);

    sun.direction           = glm::vec3(-0.5f, -1.0f, -0.3f);
    sun.ambient             = 0.4f;
    sun.shadowRadius        = 50.0f;
    sun.shadowCasterExtra   = 30.0f;
    sun.color            = {255.0f, 255.0f, 255.0f};
    sun.shadowFocus         = camera->GetPosition();
    
    clearColor = ToColor(0x0b0f14);
    LoadSkybox();
}

void MaterialScene::Update(float _dt)
{
    camera->HandleEvent(_dt);
    glm::vec3 cam = camera->GetPosition();
    sun.shadowFocus = glm::vec3(cam.x, 0, cam.z);
    Get(m_skybox).transform->SetPosition(cam);
    HandleEvent(_dt);
    Scene::Update(_dt);
}

void MaterialScene::LoadSkybox()
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
