#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#include <vector>
#include "core.h"
#include "Vertex.h"

struct Triangle
{
    std::vector<Vertex> vertices;
};

struct Particle
{
    bool alive = false;
    float lifeSpan = 0.0f;
    float lifeSpanMaxInv = 0.0f;
    glm::vec3 pos{ 0.0f };
    glm::vec3 vel{ 0.0f };
    void Update(float _dt);
};

class Emitter
{
public:
    Emitter() : m_nbrParticles(100), m_origin(glm::vec3(0.0f, 0.0f, 0.0f)) { }
    Emitter(glm::vec3 _pos, int _nbrParticles) : m_nbrParticles(_nbrParticles), m_origin(_pos) { }
    ~Emitter() = default;

    void Init();
    void Update(float _dt);
    void Render(std::vector<Vertex>& _out);

    bool m_active = true;

private:
    Triangle m_geo;
    int32_t m_nbrParticles;
    glm::vec3 m_origin;
    std::vector<Particle> m_particles;

    void SpawnParticle(Particle& _particle);
};

#endif
