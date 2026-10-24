#include "Shaders.h"
#include "Texture.h"

#include <cmath>

static Varyings BasicVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
	// Pas de matrice modele pour l'instant : la normale locale est deja en espace monde.
	out.normal = v.normal;
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

static glm::vec3 LambertPS(const Varyings& in, const Uniforms& u)
{
	glm::vec3 baseColor = u.texture ? u.texture->Sample(in.uv) : in.color;

	float normalLength2 = glm::dot(in.normal, in.normal);
	float diffuse = 1.0f;

	if (normalLength2 > 1e-8f)
	{
		glm::vec3 normal = in.normal / std::sqrt(normalLength2);
		diffuse = glm::max(glm::dot(normal, -glm::normalize(u.lightDirection)), 0.0f);
	}

	float lighting = u.ambient + (1.0f - u.ambient) * diffuse;
	return baseColor * lighting;
}

const Shader LambertShader = { BasicVS, LambertPS };
