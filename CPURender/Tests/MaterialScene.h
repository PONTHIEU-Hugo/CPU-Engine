#ifndef MATERIAL_SCENE_H_INCLUDED
#define MATERIAL_SCENE_H_INCLUDED

#include "../Scene.h"

class MaterialScene : public Scene
{
public:
    void Init() override;
    void Update(float _dt) override;
    void LoadSkybox() override;
    void HandleEvent(float _dt) override;
    
private:
    Material m_defaultMaterial;
    Material m_rockMaterial;
    Material m_sheetMaterial;
    Material m_refractMaterial;
    Material m_skyboxMaterial;
    
    Texture  m_texture;
    Texture  m_normalMap;
    Texture  m_texture2;
    Texture  m_normalMap2;
    Texture  m_texture4;
    Texture  m_normalMap4;
    
    Texture  m_arm;
    Texture  m_ao;
    Texture  m_metalness;
    Texture  m_roughness;
    Texture  m_displacement;
    
    Texture  m_arm2;
    Texture  m_ao2;
    Texture  m_metalness2;
    Texture  m_roughness2;
    Texture  m_displacement2;

    Texture  m_arm4;
    Texture  m_displacement4;
    
    Mesh     m_sphereMesh;
    Mesh     m_sphereIcoMesh;
    Mesh     m_sphereRefractionMesh;
    Mesh     m_planeMesh;
    Mesh     m_skyboxMesh;
    
    ObjectId m_skybox = 0;
};



#endif
