#include "Shaders.h"
#include "Texture.h"

#include <cmath>

// ---------------------------------------------------------------------------
// Basic / Texture / Lambert
// ---------------------------------------------------------------------------

static Varyings BasicVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	// v.position et v.normal sont deja en espace monde (Pipeline::Draw applique la matrice modele).
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
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

// ---------------------------------------------------------------------------
// Ombres (deplaces depuis ShadowMap.cpp : ShadowMap ne gere plus que la depth map)
// ---------------------------------------------------------------------------

static Varyings DepthVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
	return out;
}

const Shader DepthShader = { DepthVS, nullptr };

static float ShadowVisibility(const Varyings& in, const Uniforms& u, float cosTheta)
{
	const RenderTarget* map = u.shadowMap;
	if (!map) return 1.0f;

	if (in.lightPos.z >= 1.0f || in.lightPos.z <= -1.0f)
		return 1.0f;

	float sx = (in.lightPos.x * 0.5f + 0.5f) * (float)map->width;
	float sy = (1.0f - (in.lightPos.y * 0.5f + 0.5f)) * (float)map->height;

	int x0 = (int)std::floor(sx);
	int y0 = (int)std::floor(sy);
	if (x0 < 0 || x0 >= map->width || y0 < 0 || y0 >= map->height)
		return 1.0f;

	float c = glm::max(cosTheta, 0.05f);
	float tanTheta = glm::min(std::sqrt(1.0f - c * c) / c, 10.0f);
	float bias = u.shadowBiasConst + u.shadowBiasSlope * tanTheta;

	// PCF 3x3
	float z = in.lightPos.z - bias;
	int lit = 0;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
		{
			int x = glm::clamp(x0 + dx, 0, map->width  - 1);
			int y = glm::clamp(y0 + dy, 0, map->height - 1);
			if (z <= map->depth[(size_t)y * map->width + x])
				++lit;
		}
	return (float)lit / 9.0f;
}

static Varyings ShadowVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
	out.normal = v.normal;
	out.color  = v.color;
	out.uv     = v.uv;
	out.lightPos = glm::vec3(u.lightVP * glm::vec4(v.position, 1.0f));
	return out;
}

static glm::vec3 ShadowLambertPS(const Varyings& in, const Uniforms& u)
{
	glm::vec3 baseColor = u.texture ? u.texture->Sample(in.uv) : in.color;

	float normalLength2 = glm::dot(in.normal, in.normal);
	float diffuse = 1.0f;
	float visibility = 1.0f;

	if (normalLength2 > 1e-8f)
	{
		glm::vec3 normal = in.normal / std::sqrt(normalLength2);
		diffuse = glm::max(glm::dot(normal, -glm::normalize(u.lightDirection)), 0.0f);
		if (diffuse > 0.0f)
			visibility = ShadowVisibility(in, u, diffuse);
	}

	float lighting = u.ambient + (1.0f - u.ambient) * diffuse * visibility;
	return baseColor * lighting;
}

const Shader ShadowLambertShader = { ShadowVS, ShadowLambertPS };
