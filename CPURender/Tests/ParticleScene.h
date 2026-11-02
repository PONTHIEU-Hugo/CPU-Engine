#ifndef PARTICLE_SCENE_H_INCLUDED
#define PARTICLE_SCENE_H_INCLUDED

#include "../Scene.h"
#include "../SmokeEmitter.h"
#include "../SnowEmitter.h"
#include "../SparkleEmitter.h"

// Vitrine des emetteurs : aucun objet, fond noir, un emetteur par colonne (fumee, neige, etincelles).
class ParticleScene : public Scene
{
public:
    void Init() override;
    void Update(float _dt) override;
    void LoadSkybox() override {}
    void HandleEvent(float _dt) override;

private:
    SmokeEmitter   m_smoke;
    SnowEmitter    m_snow;
    SparkleEmitter m_sparkle;

    float m_burstTimer = 0.0f;
    bool  m_pWasDown = false;
    bool  m_bWasDown = false;
};

#endif
