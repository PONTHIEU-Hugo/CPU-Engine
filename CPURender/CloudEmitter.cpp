#ifndef CLOUD_EMITTER_CPP_INCLUDED
#define CLOUD_EMITTER_CPP_INCLUDED

#include "CloudEmitter.h"

CloudEmitter::CloudEmitter(int _max) : Emitter(_max) 
{
    settings.emissionRate = 25.0f;
    settings.lifeMin = 10.0f;
    settings.lifeMax = 16.0f;
    settings.speedMin = 0.1f;
    settings.speedMax = 0.4f;
    settings.coneAngle = 80.0f;               
    settings.radius = 80.0f;                   
    settings.acceleration = { 0.3f, 0.0f, 0.0f }; 
    settings.drag = 0.8f;
    settings.sizeStart = 6.0f;
    settings.sizeEnd = 10.0f;
    settings.colorStart = { 255.0f, 255.0f, 255.0f, 90.0f };
    settings.colorEnd   = { 205.0f, 215.0f, 230.0f, 70.0f };
}

void CloudEmitter::Integrate(Particle& p, float dt)
{
    Emitter::Integrate(p, dt);
    float ph = p.age * swayFrequency + p.seed;
    p.pos += glm::vec3(std::sin(ph), 0.0f, std::cos(ph * 1.7f)) * swayAmplitude * p.age * dt;
}

void CloudEmitter::Appearance(const Particle& p, float t, glm::vec4& c, float& s) const
{
    float age = 1.0f - t;
    float a = glm::smoothstep(0.0f, 0.1f, age) * t;

    c = LifeColor(t);
    c.a *= a * p.alpha;
    s = glm::mix(settings.sizeEnd, settings.sizeStart, t);
}

#endif