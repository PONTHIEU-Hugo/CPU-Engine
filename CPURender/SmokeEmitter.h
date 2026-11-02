#ifndef SMOKE_EMITTER_INCLUDED
#define SMOKE_EMITTER_INCLUDED

#include "ParticleSystem.h"

class SmokeEmitter : public Emitter
{
public:
    float swayAmplitude = 0.25f, swayFrequency = 1.3f;

    explicit SmokeEmitter(int _max = 500);

    BlendMode GetBlendMode() const override { return BlendMode::Alpha; }
    bool NeedsSorting() const override { return true; }

protected:
    void Integrate(Particle& p, float dt) override;
    void Appearance(const Particle& p, float t, glm::vec4& c, float& s) const override;
};

#endif // !SMOKE_EMITTER_INCLUDED



