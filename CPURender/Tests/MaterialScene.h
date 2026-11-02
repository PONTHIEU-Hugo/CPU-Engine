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
    Material m_cubeMaterial;
    Texture  m_cubeTexture;
    Texture  m_cubeNormalMap;
    Mesh     m_sphereMesh;
    Mesh     m_sphereIcoMesh;
    Mesh     m_cubeMesh;
};



#endif
