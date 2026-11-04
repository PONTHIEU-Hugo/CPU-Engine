#ifndef RENDU_SCENE_H_INCLUDED
#define RENDU_SCENE_H_INCLUDED

#include "../Scene.h"

class RenduScene : public Scene
{
public:
    void Init() override;
    void Update(float _dt) override;
    void LoadSkybox() override;
    void HandleEvent(float _dt) override;
    
private:
    Material m_roofMaterial;
    Material m_rockMaterial;
    Material m_concreteMaterial;
    Material m_woodMaterial;
    Material m_assetMaterial;
    
    Texture  m_roofTexture;
    Texture  m_rockTexture;
    Texture  m_concreteTexture;
    Texture  m_woodTexture;
    Texture  m_assetTexture;
    
    Texture  m_normalMapRoof;
    Texture  m_normalMapRock;
    Texture  m_normalMapConcrete;
    Texture  m_normalMapWood;
    
    Texture  m_armRoof;
    Texture  m_displacementRoof;
    
    Texture  m_armRock;
    Texture  m_displacementRock;
    
    Texture  m_armConcrete;
    Texture  m_displacementConcrete;

    Texture  m_armWood;
    Texture  m_displacementWood;
    
    Mesh     m_roof;
    Mesh     m_wood;
    Mesh     m_wallOfRock;
    Mesh     m_wallOfConcrete;
    Mesh     m_assets;
};



#endif
