#include "ParticleSystem.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <utility>
#include <glm/gtc/constants.hpp>

// Un seul gros pas de temps (premiere frame, fenetre deplacee) ne doit pas creer une rafale de particules.
static const float MAX_DT = 0.1f;

void Emitter::Init()
{
    m_particles.assign(m_maxParticles, Particle());
    m_alive.clear();
    m_alive.reserve(m_maxParticles);
    m_drawOrder.clear();
    m_drawOrder.reserve(m_maxParticles);
    m_free.clear();
    m_free.reserve(m_maxParticles);
    for (int i = m_maxParticles - 1; i >= 0; --i)
        m_free.push_back(i);
    m_emitAccumulator = 0.0f;
    m_mesh.vertices.clear();
    m_mesh.vertices.reserve((size_t)m_maxParticles * 6);
    m_mesh.ComputeBounds();
}

void Emitter::Clear()
{
    for (int index : m_alive)
    {
        m_particles[index].alive = false;
        m_free.push_back(index);
    }
    m_alive.clear();
    m_emitAccumulator = 0.0f;
}

int Emitter::AliveCount() const
{
    return (int)m_alive.size();
}

Particle* Emitter::Allocate()
{
    if (m_free.empty())
        return nullptr;   // pool plein

    int index = m_free.back();
    m_free.pop_back();
    m_alive.push_back(index);
    return &m_particles[index];
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

    // Compactage en place : l'ordre des vivantes est conserve, les mortes retournent dans le pool.
    size_t kept = 0;
    for (size_t i = 0; i < m_alive.size(); ++i)
    {
        int index = m_alive[i];
        Particle& particle = m_particles[index];

        particle.age += _dt;
        particle.life -= _dt;
        Integrate(particle, _dt);

        if (ShouldDie(particle))
        {
            particle.alive = false;
            m_free.push_back(index);
        }
        else
        {
            m_alive[kept++] = index;
        }
    }
    m_alive.resize(kept);

    if (m_active == false || m_particles.empty())
        return;

    m_emitAccumulator += settings.emissionRate * _dt;
    while (m_emitAccumulator >= 1.0f)
    {
        Particle* particle = Allocate();
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
        Particle* particle = Allocate();
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

    const std::vector<int>* drawOrder = &m_alive;
    if (NeedsSorting())
    {
        // Profondeur le long de l'axe camera : les plus eloignees (valeur la plus petite) d'abord.
        std::vector<std::pair<float, int>> depthAndIndex;
        depthAndIndex.reserve(m_alive.size());
        for (int index : m_alive)
            depthAndIndex.emplace_back(glm::dot(m_particles[index].pos, towardCamera), index);

        std::sort(depthAndIndex.begin(), depthAndIndex.end());

        m_drawOrder.clear();
        for (const auto& entry : depthAndIndex)
            m_drawOrder.push_back(entry.second);
        drawOrder = &m_drawOrder;
    }

    m_mesh.vertices.clear();

    // Boite englobante cumulee pendant la construction (evite un second parcours des sommets).
    const glm::vec3 cameraExtent = glm::abs(right) + glm::abs(up);
    glm::vec3 boundsMin(FLT_MAX);
    glm::vec3 boundsMax(-FLT_MAX);

    for (int index : *drawOrder)
    {
        const Particle& particle = m_particles[index];

        float t = glm::clamp(particle.life * particle.lifeInv, 0.0f, 1.0f);   // 1 a la naissance, 0 a la mort

        glm::vec4 color(0.0f);
        float size = 0.0f;
        Appearance(particle, t, color, size);

        if (size <= 0.0f || color.a <= 0.0f)
            continue;

        boundsMin = glm::min(boundsMin, particle.pos - cameraExtent * size);
        boundsMax = glm::max(boundsMax, particle.pos + cameraExtent * size);

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

    if (m_mesh.vertices.empty())
        m_mesh.localBounds = AABB();
    else
        m_mesh.localBounds = { boundsMin, boundsMax };
}