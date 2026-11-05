#ifndef RENDU_SCENE_H_INCLUDED
#define RENDU_SCENE_H_INCLUDED

#include "../CloudEmitter.h"
#include "../Scene.h"
#include "../SnowEmitter.h"
#include "../TerrainGenerator.h"
#include "../OceanWaves.h"

struct Boat
{
    void Init(Scene& scene);
    void TurnAround(float _dt);
    
    Mesh barqueMesh;
    Mesh supportMesh;
    Mesh floorMesh;
    Mesh nappeMesh;
    Mesh cabineMesh;
    Mesh motorMesh;
    
    Material barqueMaterial;
    Material supportMaterial;
    Material floorMaterial;
    Material nappeMaterial;
    Material cabineMaterial;
    Material motorMaterial;
    
    Transform* m_transform = nullptr;
};

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
    void BuildBoat();

    Model m_dolphinModel;
    Mesh m_dolphin;
    Animation*  m_animDolphin = nullptr;
    Animator*   m_animator = nullptr;
    Material m_dolphinMaterial;
    Transform* m_dolphinTransform = nullptr;
    
    Boat m_boat;
    
    Material m_groundMaterial;
    Material m_roofMaterial;
    Material m_rockMaterial;
    Material m_concreteMaterial;
    Material m_woodMaterial;
    Material m_assetMaterial;
    
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
