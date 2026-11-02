#ifndef SPARKLE_EMITTER_H_INCLUDED
#define SPARKLE_EMITTER_H_INCLUDED

#include "ParticleSystem.h"

class SparkleEmitter : public Emitter
{
public:
    float flickerSpeed = 50.0f;

    explicit SparkleEmitter(int _max = 1000);   // preset : sphère, gravité, drag 2

    BlendMode GetBlendMode() const override { return BlendMode::Additive; }

protected:
    void Appearance(const Particle& p, float t, glm::vec4& c, float& s) const override;
};

#endif // !SPARKLE_EMITTER_H_INCLUDED