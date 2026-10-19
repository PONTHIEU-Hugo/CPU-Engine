#ifndef PARTICLE_SYSTEM_CPP_INCLUDED
#define PARTICLE_SYSTEM_CPP_INCLUDED

#include "ParticleSystem.h"
#include "core.h"
#include <glm/gtc/constants.hpp>

void Particle::Update(float _dt, i32vec2& _colors)
{
    if ( alive == false )
        return;
    
    pos += vel * _dt;
    
    lifeSpan -= _dt;
    if ( lifeSpan <= 0.0f )
        alive = false;
}

void Emitter::Init()
{
    float size = 2.5f;
    std::vector<Vertex> vertices = {{-size, size}, {0.0f, -size}, { size, size}};
    m_geo.vertices = vertices;
    for (int i = 0; i < m_nbrParticles; i++)
    {
        Particle particle;
        SpawnParticle(particle);
        m_particles.push_back(particle);
    }
}

void Emitter::Update(float _dt)
{
    for (auto& element : m_particles)
    {
        element.Update(_dt, m_colors);
        if ( element.alive == false && m_active)
            SpawnParticle(element);
    }
}

void Emitter::Render(FinalFrameBuffer& frameBufferOut)
{
    for (auto& element : m_particles)
    {
        if (element.alive)
        {
            m_geo.pos = element.pos;
            int index = FIRE_PALETTE_SIZE * element.lifeSpan * element.lifeSpanMaxInv;
            m_geo.color = (uint32_t)fire_palette[index];
            Rasterization(frameBufferOut, m_geo);
        }
    }
}

void Emitter::SpawnParticle(Particle& _particle)
{
    _particle.pos = m_origin;
    
    float angle = getRandomFloat(0.0f, 2.0f* pi<float>());
    vec2 dir = vec2(cosf(angle), sinf(angle));
    float speed = getRandomFloat(2.0f, 15.0f);
    _particle.vel = dir * speed;
    
    _particle.lifeSpan = getRandomFloat(1.0f, 2.0f);
    _particle.lifeSpanMaxInv = 1 / _particle.lifeSpan;
    _particle.alive = true;
}

#endif
