#ifndef MAIN_SCENE_H_INCLUDED
#define MAIN_SCENE_H_INCLUDED

#include "Scene.h"
#include "Texture.h"
#include "TerrainGenerator.h"
#include "SmokeEmitter.h"
#include "SnowEmitter.h"
#include "SparkleEmitter.h"

class MainScene : public Scene
{
public:
    MainScene() = default;
    ~MainScene() = default;
    void Init() override;
    void Update(float _dt) override;
    void LoadSkybox() override;
    void HandleEvent(float _dt) override;
    
private:
    
    Mesh     m_cubeMesh;
    Mesh     m_carMesh;
    Mesh     m_wheelAvMesh;
    Mesh     m_wheelArMesh;
    Mesh     m_groundMesh;
    Texture  m_carTexture;
    Texture  m_cubeTexture;

    Material m_defaultMaterial;
    Material m_cubeMaterial;
    Material m_carMaterial;
    Material m_groundMaterial;
    
    ObjectId m_car = 0;
    ObjectId m_wheelAr = -1;
    ObjectId m_wheelAv = -1;

    bool     m_pWasDown = false;
    bool     m_bWasDown = false;

    SmokeEmitter   m_smoke;     // pot d'echappement
    SnowEmitter    m_snow;      // suit la camera
    SparkleEmitter m_sparkle;   // sur le toit de la voiture (B = rafale)

    TerrainGenerator m_Terrain;
};


#endif
