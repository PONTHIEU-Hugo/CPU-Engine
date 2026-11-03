#include "SnowEmitter.h"

#include <cmath>

SnowEmitter::SnowEmitter(int _max) : Emitter(_max)
{
	settings.emissionRate = 300.0f;
	settings.lifeMin = 8.0f;
	settings.lifeMax = 12.0f;
	settings.speedMin = 1.0f;
	settings.speedMax = 2.0f;
	settings.coneAngle = 5.0f;
	settings.radius = 25.0f;
	settings.direction = { 0.0f, -1.0f, 0.0f };
	settings.sizeStart = 0.08f;
	settings.sizeEnd = 0.08f;
}

void SnowEmitter::Integrate(Particle& p, float dt)
{
	Emitter::Integrate(p, dt);
	float ph = p.age * swayFrequency + p.seed;
	p.pos += glm::vec3(std::sin(ph), 0.0f, std::cos(ph * 1.3f)) * swayAmplitude * dt;
}

void SnowEmitter::Appearance(const Particle& p, float t, glm::vec4& c, float& s) const
{
	float fade = glm::smoothstep(0.0f, 0.1f, t) * glm::smoothstep(0.0f, 0.05f, 1.0f - t);
	c = LifeColor(t);
	c.a *= p.alpha * fade;
	s = glm::mix(settings.sizeEnd, settings.sizeStart, t);
}
