#ifndef PARTICLE_SYSTEM_H_INCLUDED
#define PARTICLE_SYSTEM_H_INCLUDED

#include <vector>
#include "core.h"
#include "Mesh.h"

enum class BlendMode
{
    Alpha,      // SrcAlpha / InvSrcAlpha, necessite un tri back-to-front
    Additive    // SrcAlpha / One, ordre indifferent
};

struct EmitterSettings
{
    glm::vec3 origin{ 0.0f };
    float emissionRate = 100.0f;                // particules creees par seconde
    float lifeMin = 1.0f;                       // duree de vie (secondes)
    float lifeMax = 2.0f;
    float speedMin = 1.0f;                      // vitesse initiale le long du cone
    float speedMax = 2.0f;
    float coneAngle = 20.0f;                    // demi-angle du cone (degres), 180 = sphere
    float radius = 0.0f;                        // rayon du disque d'emission
    glm::vec3 direction{ 0.0f, 1.0f, 0.0f };    // axe du cone
    glm::vec3 velocity{ 0.0f };                 // vitesse ajoutee a toutes les particules
    glm::vec3 acceleration{ 0.0f };
    float drag = 0.0f;                          // amortissement de la vitesse (1/s)
    float sizeStart = 0.1f;
    float sizeEnd = 0.1f;
    glm::vec4 colorStart{ 255.0f };             
    glm::vec4 colorEnd{ 255.0f };               
};

struct Particle
{
    bool alive = false;
    float life = 0.0f;                          // temps restant
    float lifeInv = 0.0f;                       // 1 / duree de vie totale
    float age = 0.0f;                           // temps ecoule depuis la naissance
    float seed = 0.0f;                          // phase aleatoire [0, 2pi]
    float alpha = 1.0f;                         // fondu radial calcule au spawn
    glm::vec3 pos{ 0.0f };
    glm::vec3 vel{ 0.0f };
};

class Emitter
{
public:
    explicit Emitter(int _maxParticles = 1000) : m_maxParticles(_maxParticles) {}
    virtual ~Emitter() = default;

    Emitter(const Emitter&) = delete;
    Emitter& operator=(const Emitter&) = delete;

    EmitterSettings settings;

    void Init();
    void Update(float _dt);
    void BuildQuads(const glm::mat4& _view);   // remplit le mesh (un quad par particule vivante)
    void Burst(int _count);                    // emet _count particules immediatement
    void Clear();                              // tue toutes les particules

    void SetActive(bool _active) { m_active = _active; }
    bool IsActive() const { return m_active; }
    void SetOrigin(const glm::vec3& _origin) { settings.origin = _origin; }

    int AliveCount() const;
    int MaxParticles() const { return m_maxParticles; }
    const Mesh& GetMesh() const { return m_mesh; }

    virtual BlendMode GetBlendMode() const = 0;
    virtual bool NeedsSorting() const { return false; }

protected:

    // Degrade colorStart -> colorEnd ; _t vaut 1 a la naissance et 0 a la mort.
    glm::vec4 LifeColor(float _t) const { return glm::mix(settings.colorEnd, settings.colorStart, _t); }

    virtual void OnSpawn(Particle& /*_particle*/) {}
    virtual void Integrate(Particle& _particle, float _dt);
    virtual bool ShouldDie(const Particle& _particle) const { return _particle.life <= 0.0f; }
    virtual void Appearance(const Particle& _particle, float _t, glm::vec4& _color, float& _size) const = 0;

private:
    void Spawn(Particle& _particle);
    Particle* Allocate();                      // prend une particule libre et la marque vivante, nullptr si le pool est plein
    static void BuildBase(const glm::vec3& _n, glm::vec3& _t, glm::vec3& _b);

    int m_maxParticles;
    bool m_active = true;
    float m_emitAccumulator = 0.0f;            // fraction de particule a emettre
    std::vector<Particle> m_particles;
    std::vector<int> m_alive;                  // indices des particules vivantes (seules celles-ci sont parcourues)
    std::vector<int> m_free;                   // indices des particules libres (pile)
    std::vector<int> m_drawOrder;              // copie de m_alive triee, utilisee seulement si NeedsSorting()
    Mesh m_mesh;
};

#endif
