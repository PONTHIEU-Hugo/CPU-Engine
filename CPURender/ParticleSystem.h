#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#include <vector>
#include "core.h"
#include "Mesh.h"

struct EmitterSettings
{
    glm::vec3 origin{ 0.0f };
    float     emissionRate = 200.0f;           // particules creees par seconde
    float     lifeMin = 0.6f;                  // duree de vie (secondes)
    float     lifeMax = 1.2f;
    float     speedMin = 2.0f;                 // vitesse initiale
    float     speedMax = 5.0f;
    float     coneAngle = 25.0f;               
    glm::vec3 acceleration{ 0.0f, 6.0f, 0.0f };   
    float     sizeStart = 0.35f;               
    float     sizeEnd = 0.05f;                 
};

struct Particle
{
    bool      alive = false;
    float     life = 0.0f;                     // temps restant
    float     lifeInv = 0.0f;                  // 1 / duree de vie totale
    glm::vec3 pos{ 0.0f };
    glm::vec3 vel{ 0.0f };

    void Update(float _dt, const glm::vec3& _acceleration);
};

class Emitter
{
public:
    explicit Emitter(int _maxParticles = 1000) : m_maxParticles(_maxParticles) {}

    EmitterSettings settings;

    void Init();                                       
    void Update(float _dt);                            
    void BuildQuads(const glm::mat4& _view); // remplit le mesh (un quad par particule vivante)

    void SetActive(bool _active) { m_active = _active; }
    bool IsActive() const { return m_active; }
    void SetOrigin(const glm::vec3& _origin) { settings.origin = _origin; }
    void Clear(); // tue toutes les particules

    int  AliveCount() const;
    int  MaxParticles() const { return m_maxParticles; }
    const Mesh& GetMesh() const { return m_mesh; }

private:
    void Spawn(Particle& _particle);
    Particle* FindDead();

    int                   m_maxParticles;
    bool                  m_active = true;
    float                 m_emitAccumulator = 0.0f; // fraction de particule a emettre
    int                   m_cursor = 0; // ou reprendre la recherche d'une particule morte
    std::vector<Particle> m_particles;
    Mesh                  m_mesh;                
};

#endif
