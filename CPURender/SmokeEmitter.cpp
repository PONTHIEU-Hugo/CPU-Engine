#include "SmokeEmitter.h"

SmokeEmitter::SmokeEmitter(int _max) : Emitter(_max) 
{
    settings.emissionRate = 40.0f;
    settings.lifeMin = 2.5f;
    settings.lifeMax = 4.0f;
    settings.speedMin = 0.3f;
    settings.speedMax = 0.8f;
    settings.coneAngle = 15.0f;
    settings.radius = 0.2f;
    settings.acceleration = { 0.6f, 0.3f, 0.0f };
    settings.drag = 0.6f;
    settings.sizeStart = 0.5f;
    settings.sizeEnd = 1.2f;
    settings.colorStart = { 70.0f, 70.0f, 70.0f, 128.0f };
    settings.colorEnd = { 170.0f, 170.0f, 170.0f, 128.0f };
}

void SmokeEmitter::Integrate(Particle& p, float dt)
{
    Emitter::Integrate(p, dt);
    float ph = p.age * swayFrequency + p.seed;
    p.pos += glm::vec3(std::sin(ph), 0.0f, std::cos(ph * 1.7f)) * swayAmplitude * p.age * dt;
}

void SmokeEmitter::Appearance(const Particle& p, float t, glm::vec4& c, float& s) const
{
    float age = 1.0f - t;
    float a = glm::smoothstep(0.0f, 0.1f, age) * t;

    c = LifeColor(t);
    c.a *= a * p.alpha;
    s = glm::mix(settings.sizeEnd, settings.sizeStart, t);
}
