#include "Shaders.h"

static Varyings BasicVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
	out.color = v.color;

	return out;
}

static glm::vec3 BasicPS(const Varyings& in, const Uniforms& u)
{
	return in.color;	
}

const Shader BasicShader = { BasicVS, BasicPS };
