#include "SnowEmitter.h"

SnowEmitter::SnowEmitter(int _max) : Emitter(_max)
{

}

void SnowEmitter::Integrate(Particle& p, float dt)
{
	Emitter::Integrate(p, dt);
}

void SnowEmitter::Appearance(const Particle& p, float t, glm::vec4& c, float& s) const
{
	c = { 255.0f, 255.0f, 255.0f, 255.0f * p.alpha };
	s = glm::mix(settings.sizeEnd, settings.sizeStart, t);
}
