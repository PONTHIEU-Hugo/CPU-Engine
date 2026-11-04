#ifndef CLOUD_EMITTER_INCLUDED
#define CLOUD_EMITTER_INCLUDED

#include "ParticleSystem.h"

class CloudEmitter : public Emitter
{
public:
    float swayAmplitude = 0.15f, swayFrequency = 0.3f;

    explicit CloudEmitter(int _max = 500);

    EmitterSettings& GetSettings() { return settings; }
    BlendMode GetBlendMode() const override { return BlendMode::Alpha; }
    bool NeedsSorting() const override { return true; }
protected:
    void Integrate(Particle& p, float dt) override;
    void Appearance(const Particle& p, float t, glm::vec4& c, float& s) const override;
};

#endif



