#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#include "core.h"

struct Triangle
{
    std::vector<Vertex> vertices;
};

struct Particle
{
    bool alive;
    float lifeSpan;
    float lifeSpanMaxInv;
    vec3 pos;
    vec3 vel;
    void Update(float _dt);
};

class Emitter
{
public:
    Emitter() : m_origin(vec3(0.0f, 0.0f, 0.0f)), m_nbrParticles(100) { }
    Emitter(vec3 _pos, int _nbrParticles) : m_origin(_pos), m_nbrParticles(_nbrParticles) { }
    ~Emitter() = default;
    
    void Init();
    void Update(float _dt);
    void Render(std::vector<Vertex>& _out);
    
    bool m_active = true;
    
private:
    Triangle m_geo;
    int32_t m_nbrParticles;
    vec3 m_origin;
    std::vector<Particle> m_particles;
    
    void SpawnParticle(Particle& _particle);
};




#endif