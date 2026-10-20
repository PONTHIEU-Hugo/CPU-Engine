#ifndef PARTICLE_SYSTEM_CPP_INCLUDED
#define PARTICLE_SYSTEM_CPP_INCLUDED

#include "ParticleSystem.h"
#include "core.h"
#include <glm/gtc/constants.hpp>

void Particle::Update(float _dt)
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
    float size = 1.0f;
    std::vector<Vertex> vertices = {
        {{-size, size, 0}, ToColor(0xFFFFFFFF)},
        {{size, size, 0},  ToColor(0xFFFFFFFF)},
        {{0.0f, -size, 0},  ToColor(0xFFFFFFFF)}
    };
    
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
        element.Update(_dt);
        if ( element.alive == false && m_active)
            SpawnParticle(element);
    }
}

void Emitter::Render(std::vector<Vertex>& _out)
{
    for (auto& element : m_particles)
    {
        if (element.alive)
        {
            float t = element.lifeSpan * element.lifeSpanMaxInv;
            int index = (int)(t * (FIRE_PALETTE_SIZE - 1));
            index = clamp(index, 0, FIRE_PALETTE_SIZE - 1);
            vec3 color = ToColor(fire_palette[index]);
            
            for (const Vertex& base : m_geo.vertices)
            {
                Vertex v;
                v.position = base.position + element.pos;
                v.color = color;
                _out.push_back(v);
            }
        }
    }
}

void Emitter::SpawnParticle(Particle& _particle)
{
    _particle.pos = m_origin;
    
    float angle = getRandomFloat(0.0f, 2.0f* pi<float>());
    vec3 dir = vec3(cosf(angle), sinf(angle), 0.0f);
    float speed = getRandomFloat(2.0f, 15.0f);
    _particle.vel = dir * speed;
    
    _particle.lifeSpan = getRandomFloat(1.0f, 2.0f);
    _particle.lifeSpanMaxInv = 1 / _particle.lifeSpan;
    _particle.alive = true;
}

#endif
