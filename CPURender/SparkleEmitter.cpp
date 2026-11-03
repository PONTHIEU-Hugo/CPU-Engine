#include "SparkleEmitter.h"

#include <cmath>

SparkleEmitter::SparkleEmitter(int _max) : Emitter(_max)
{
    settings.emissionRate = 80.0f;
    settings.lifeMin = 0.5f;
    settings.lifeMax = 1.2f;
    settings.speedMin = 2.0f;
    settings.speedMax = 5.0f;
    settings.coneAngle = 180.0f;
    settings.radius = 0.0f;
    settings.acceleration = { 0.0f, -9.8f, 0.0f };
    settings.drag = 2.0f;
    settings.sizeStart = 0.12f;
    settings.sizeEnd = 0.02f;
    settings.colorStart = { 255.0f, 230.0f, 150.0f, 255.0f };   // blanc-jaune -> orange
    settings.colorEnd = { 255.0f, 120.0f, 30.0f, 255.0f };
}

void SparkleEmitter::Appearance(const Particle& p, float t, glm::vec4& c, float& s) const
{
    float flicker = 0.6f + 0.4f * std::sin(p.age * flickerSpeed + p.seed);

    c = LifeColor(t);
    c.a *= t * flicker;
    s = glm::mix(settings.sizeEnd, settings.sizeStart, t);
}
