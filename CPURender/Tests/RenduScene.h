#ifndef RENDU_SCENE_H_INCLUDED
#define RENDU_SCENE_H_INCLUDED

#include "../CloudEmitter.h"
#include "../Scene.h"
#include "../SnowEmitter.h"
#include "../TerrainGenerator.h"
#include "../OceanWaves.h"

class RenduScene : public Scene
{
public:
    void Init() override;
    void Update(float _dt) override;
    void LoadSkybox() override;
    void HandleEvent(float _dt) override;
    
private:
    void Move(glm::vec3 dir);
    
    enum class Weather { Clear, Cloudy, Snowy };
    
    void SetWeather(Weather w);
    
    Material m_groundMaterial;
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
    
    Mesh     m_ground;
    Mesh     m_roof;
    Mesh     m_wood;
    Mesh     m_wallOfRock;
    Mesh     m_wallOfConcrete;
    Mesh     m_assets;
    
    SnowEmitter*    m_snow = nullptr;
    CloudEmitter*   m_cloud = nullptr;
    
    TerrainGenerator m_Terrain;
    ObjectId m_oceanID = 0;
    OceanWaves m_ocean;
    Transform* m_oceanTransform = nullptr;
    int m_oceanAmplitude = 0;
    
    ObjectId m_roofID;
    ObjectId m_wallOfRockID;
    ObjectId m_wallOfConcreteID;
    ObjectId m_woodID;
    ObjectId m_assetsID;
};



#endif
