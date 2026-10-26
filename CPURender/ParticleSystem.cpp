#include "ParticleSystem.h"

#include <glm/gtc/constants.hpp>

static const int FIRE_PALETTE_SIZE = 37;

static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
    0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
    0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
    0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
    0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
    0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
    0xEFEFC7, 0xFFFFFF
};

void Particle::Update(float _dt)
{
    if (alive == false)
        return;

    pos += vel * _dt;

    lifeSpan -= _dt;
    if (lifeSpan <= 0.0f)
        alive = false;
}

void Emitter::Init()
{
    const float size = 0.1f;
    const glm::vec3 white = ToColor(0xFFFFFF);

    auto make = [&](float x, float y)
    {
        Vertex v;
        v.position = { x, y, 0.0f };
        v.color    = white;   // avant : l'init agregee {pos, couleur} remplissait la normale
        return v;
    };

    m_geo.vertices = {
        make(-size, size), make(0.0f, -size), make(size, size),
        make(-size, size), make(size, size),  make(0.0f, -size),
    };

    m_particles.clear();
    m_particles.reserve(m_nbrParticles);
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
        if (element.alive == false && m_active)
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
            index = glm::clamp(index, 0, FIRE_PALETTE_SIZE - 1);
            glm::vec3 color = ToColor(fire_palette[index]);

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

    float angle = RANDOM::GetRandomFloat(0.0f, 2.0f * glm::pi<float>());
    glm::vec3 dir = glm::vec3(cosf(angle), sinf(angle), 0.0f);
    float speed = RANDOM::GetRandomFloat(2.0f, 15.0f);
    _particle.vel = dir * speed;

    _particle.lifeSpan = RANDOM::GetRandomFloat(1.0f, 2.0f);
    _particle.lifeSpanMaxInv = 1.0f / _particle.lifeSpan;
    _particle.alive = true;
}
