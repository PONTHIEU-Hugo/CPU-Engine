#include "ParticleSystem.h"

#include <cmath>
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

// Un seul gros pas de temps (premiere frame, fenetre deplacee) ne doit pas creer une rafale de particules.
static const float MAX_DT = 0.1f;

void Particle::Update(float _dt, const glm::vec3& _acceleration)
{
    if (alive == false)
        return;

    vel += _acceleration * _dt;
    pos += vel * _dt;

    life -= _dt;
    if (life <= 0.0f)
        alive = false;
}

void Emitter::Init()
{
    m_particles.assign(m_maxParticles, Particle());
    m_emitAccumulator = 0.0f;
    m_cursor = 0;
    m_mesh.vertices.clear();
    m_mesh.vertices.reserve((size_t)m_maxParticles * 6);
    m_mesh.ComputeBounds();
}

void Emitter::Clear()
{
    for (Particle& particle : m_particles)
        particle.alive = false;
    m_emitAccumulator = 0.0f;
}

int Emitter::AliveCount() const
{
    int count = 0;
    for (const Particle& particle : m_particles)
        count += particle.alive ? 1 : 0;
    return count;
}

Particle* Emitter::FindDead()
{
    for (int i = 0; i < m_maxParticles; ++i)
    {
        int index = (m_cursor + i) % m_maxParticles;
        if (m_particles[index].alive == false)
        {
            m_cursor = (index + 1) % m_maxParticles;
            return &m_particles[index];
        }
    }
    return nullptr;   // pool plein
}

void Emitter::Update(float _dt)
{
    _dt = glm::min(_dt, MAX_DT);

    for (Particle& particle : m_particles)
        particle.Update(_dt, settings.acceleration);

    if (m_active == false || m_particles.empty())
        return;

    m_emitAccumulator += settings.emissionRate * _dt;
    while (m_emitAccumulator >= 1.0f)
    {
        Particle* particle = FindDead();
        if (particle == nullptr)
        {
            m_emitAccumulator = 0.0f;
            break;
        }
        Spawn(*particle);
        m_emitAccumulator -= 1.0f;
    }
}

void Emitter::Spawn(Particle& _particle)
{
    // Direction uniforme dans un cone autour de +Y.
    float cosMax = std::cos(glm::radians(settings.coneAngle));
    float cosTheta = RANDOM::GetRandomFloat(cosMax, 1.0f);
    float sinTheta = std::sqrt(glm::max(1.0f - cosTheta * cosTheta, 0.0f));
    float phi = RANDOM::GetRandomFloat(0.0f, 2.0f * glm::pi<float>());
    glm::vec3 dir(sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi));

    _particle.pos = settings.origin;
    _particle.vel = dir * RANDOM::GetRandomFloat(settings.speedMin, settings.speedMax);

    _particle.life = RANDOM::GetRandomFloat(settings.lifeMin, settings.lifeMax);
    _particle.lifeInv = 1.0f / _particle.life;
    _particle.alive = true;
}

void Emitter::BuildQuads(const glm::mat4& _view)
{
    // Axes de la camera en espace monde = lignes de la partie rotation de la matrice de vue.
    const glm::vec3 right(_view[0][0], _view[1][0], _view[2][0]);
    const glm::vec3 up(_view[0][1], _view[1][1], _view[2][1]);
    const glm::vec3 towardCamera(_view[0][2], _view[1][2], _view[2][2]);

    static const glm::vec2 corners[4]    = { { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 1.0f } };
    static const glm::vec2 corners_uv[4] = { { 0.0f, 1.0f },   { 1.0f, 1.0f },  { 1.0f, 0.0f }, { 0.0f, 0.0f } };

    m_mesh.vertices.clear();

    for (const Particle& particle : m_particles)
    {
        if (particle.alive == false)
            continue;

        float t = glm::clamp(particle.life * particle.lifeInv, 0.0f, 1.0f);   // 1 a la naissance, 0 a la mort
        int index = glm::clamp((int)(t * (FIRE_PALETTE_SIZE - 1)), 0, FIRE_PALETTE_SIZE - 1);

        // ToColor lit l'alpha dans les bits 24-31 : la palette n'en a pas (0xRRGGBB), on force l'opacite.
        glm::vec4 color = ToColor(fire_palette[index]);
        color.a = 255.0f;
        float size = glm::mix(settings.sizeEnd, settings.sizeStart, t);

        Vertex quad[4];
        for (int i = 0; i < 4; ++i)
        {
            quad[i].position = particle.pos + right * (corners[i].x * size) + up * (corners[i].y * size);
            quad[i].normal = towardCamera;
            quad[i].color = color;
            quad[i].uv = corners_uv[i];
        }

        m_mesh.vertices.push_back(quad[0]);
        m_mesh.vertices.push_back(quad[1]);
        m_mesh.vertices.push_back(quad[2]);
        m_mesh.vertices.push_back(quad[0]);
        m_mesh.vertices.push_back(quad[2]);
        m_mesh.vertices.push_back(quad[3]);
    }

    m_mesh.ComputeBounds();
}
