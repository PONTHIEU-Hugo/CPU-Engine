#include "Shaders.h"
#include "Texture.h"

static Varyings BasicVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
	out.color = v.color;
	out.uv = v.uv;

	return out;
}

static glm::vec3 BasicPS(const Varyings& in, const Uniforms& u)
{
	return in.color;
}

const Shader BasicShader = { BasicVS, BasicPS };

static glm::vec3 TexturePS(const Varyings& in, const Uniforms& u)
{
	if (!u.texture)
		return in.color;

	return u.texture->Sample(in.uv);
}

const Shader TextureShader = { BasicVS, TexturePS };
