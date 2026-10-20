#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#include "core.h"

struct Particle
{
    bool alive;
    float lifeSpan;
    float lifeSpanMaxInv;
    vec2 pos;
    vec2 vel;
    void Update(float _dt);
};

class Emitter
{
public:
    Emitter() : m_origin(vec2(0.0f, 0.0f)), m_colors(0xFFFFFF, 0x070707), m_nbrParticles(100) { }
    Emitter(vec2 _pos, i32vec2 _colors, int _nbrParticles) : m_origin(_pos), m_colors(_colors), m_nbrParticles(_nbrParticles) { }
    ~Emitter() = default;
    
    void Init();
    void Update(float _dt);
    void Render(FinalFrameBuffer& frameBufferOut);
    
    bool m_active = true;
    
private:
    Triangle m_geo;
    int32_t m_nbrParticles;
    vec2 m_origin;
    i32vec2 m_colors;
    std::vector<Particle> m_particles;
    
    void SpawnParticle(Particle& _particle);
};




#endif