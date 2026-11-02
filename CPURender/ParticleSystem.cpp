#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>

// Un seul gros pas de temps (premiere frame, fenetre deplacee) ne doit pas creer une rafale de particules.
static const float MAX_DT = 0.1f;

void Emitter::Init()
{
    m_particles.assign(m_maxParticles, Particle());
    m_drawOrder.clear();
    m_drawOrder.reserve(m_maxParticles);
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

void Emitter::Integrate(Particle& _particle, float _dt)
{
    _particle.vel += settings.acceleration * _dt;
    _particle.vel *= std::exp(-settings.drag * _dt);   // independant du framerate
    _particle.pos += _particle.vel * _dt;
}

void Emitter::Update(float _dt)
{
    _dt = glm::min(_dt, MAX_DT);

    for (Particle& particle : m_particles)
    {
        if (particle.alive == false)
            continue;

        particle.age += _dt;
        particle.life -= _dt;
        Integrate(particle, _dt);

        if (ShouldDie(particle))
            particle.alive = false;
    }

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

void Emitter::Burst(int _count)
{
    for (int i = 0; i < _count; ++i)
    {
        Particle* particle = FindDead();
        if (particle == nullptr)
            break;
        Spawn(*particle);
    }
}

void Emitter::Spawn(Particle& _particle)
{
    glm::vec3 n = (glm::dot(settings.direction, settings.direction) > 0.0f) ? glm::normalize(settings.direction) : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 t, b;
    BuildBase(n, t, b);

    // Direction uniforme dans un cone autour de n.
    float cosMax = std::cos(glm::radians(settings.coneAngle));
    float cosTheta = RANDOM::GetRandomFloat(cosMax, 1.0f);
    float sinTheta = std::sqrt(glm::max(1.0f - cosTheta * cosTheta, 0.0f));
    float phi = RANDOM::GetRandomFloat(0.0f, 2.0f * glm::pi<float>());
    glm::vec3 dir = t * (sinTheta * std::cos(phi)) + n * cosTheta + b * (sinTheta * std::sin(phi));

    // Position uniforme dans un disque perpendiculaire a n.
    float r = settings.radius * std::sqrt(RANDOM::GetRandomFloat(0.0f, 1.0f));
    float alpha = RANDOM::GetRandomFloat(0.0f, 2.0f * glm::pi<float>());
    glm::vec3 offset = t * (r * std::cos(alpha)) + b * (r * std::sin(alpha));

    // Fondu radial : plus la particule est loin de l'axe, plus elle est transparente.
    float theta = std::acos(glm::clamp(cosTheta, -1.0f, 1.0f));
    float angleT = (settings.coneAngle > 0.0f) ? theta / glm::radians(settings.coneAngle) : 0.0f;
    float radiusT = (settings.radius > 0.0f) ? r / settings.radius : 0.0f;
    float axisT = glm::clamp(glm::max(angleT, radiusT), 0.0f, 1.0f);
    _particle.alpha = 1.0f - axisT * axisT;

    _particle.pos = settings.origin + offset;
    _particle.vel = settings.velocity + dir * RANDOM::GetRandomFloat(settings.speedMin, settings.speedMax);

    _particle.life = glm::max(RANDOM::GetRandomFloat(settings.lifeMin, settings.lifeMax), 0.001f);
    _particle.lifeInv = 1.0f / _particle.life;
    _particle.age = 0.0f;
    _particle.seed = RANDOM::GetRandomFloat(0.0f, 2.0f * glm::pi<float>());
    _particle.alive = true;

    OnSpawn(_particle);
}

// Base orthonormee a partir d'une normale (Duff et al. 2017).
void Emitter::BuildBase(const glm::vec3& _n, glm::vec3& _t, glm::vec3& _b)
{
    float sign = std::copysign(1.0f, _n.z);
    float a = -1.0f / (sign + _n.z);
    float c = _n.x * _n.y * a;
    _t = glm::vec3(1.0f + sign * _n.x * _n.x * a, sign * c, -sign * _n.x);
    _b = glm::vec3(c, sign + _n.y * _n.y * a, -_n.y);
}

void Emitter::BuildQuads(const glm::mat4& _view)
{
    // Axes de la camera en espace monde = lignes de la partie rotation de la matrice de vue.
    const glm::vec3 right(_view[0][0], _view[1][0], _view[2][0]);
    const glm::vec3 up(_view[0][1], _view[1][1], _view[2][1]);
    const glm::vec3 towardCamera(_view[0][2], _view[1][2], _view[2][2]);

    static const glm::vec2 corners[4] = { { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 1.0f } };
    static const glm::vec2 corners_uv[4] = { { 0.0f, 1.0f },   { 1.0f, 1.0f },  { 1.0f, 0.0f }, { 0.0f, 0.0f } };

    m_drawOrder.clear();
    for (int i = 0; i < (int)m_particles.size(); ++i)
        if (m_particles[i].alive)
            m_drawOrder.push_back(i);

    if (NeedsSorting())
    {
        std::sort(m_drawOrder.begin(), m_drawOrder.end(), [&](int _a, int _b)
            {
                return glm::dot(m_particles[_a].pos, towardCamera) < glm::dot(m_particles[_b].pos, towardCamera);
            });
    }

    m_mesh.vertices.clear();

    for (int index : m_drawOrder)
    {
        const Particle& particle = m_particles[index];

        float t = glm::clamp(particle.life * particle.lifeInv, 0.0f, 1.0f);   // 1 a la naissance, 0 a la mort

        glm::vec4 color(0.0f);
        float size = 0.0f;
        Appearance(particle, t, color, size);

        if (size <= 0.0f || color.a <= 0.0f)
            continue; 

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