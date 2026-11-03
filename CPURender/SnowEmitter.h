#ifndef SNOW_EMITTER_H_INCLUDED
#define SNOW_EMITTER_H_INCLUDED

#include "ParticleSystem.h"

class SnowEmitter : public Emitter
{
public:
    float swayAmplitude = 0.4f, swayFrequency = 1.5f;
    float groundHeight = 0.0f;

    explicit SnowEmitter(int _max = 4000);

    BlendMode GetBlendMode() const override { return BlendMode::Alpha; }

protected:
    void OnSpawn(Particle& p) override { p.alpha = 1.0f; }   // pas de fondu radial
    void Integrate(Particle& p, float dt) override;          // chute + sway
    bool ShouldDie(const Particle& p) const override
    {
        return p.life <= 0.0f || p.pos.y < groundHeight;
    }
    void Appearance(const Particle& p, float t, glm::vec4& c, float& s) const override;
};

#endif // !SNOW_EMITTER_H_INCLUDED


