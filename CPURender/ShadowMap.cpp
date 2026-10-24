#ifndef SHADOW_MAP_CPP_INCLUDED
#define SHADOW_MAP_CPP_INCLUDED

#include "ShadowMap.h"
#include "Texture.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

static const float kDepthRangeInRadii = 4.0f;

static glm::mat4 ComputeLightVP(const glm::vec3& lightDir, const AABB& bounds, float radius)
{
	glm::vec3 center = (bounds.min + bounds.max) * 0.5f;

	// Éviter un "haut" parallèle à la direction de la lumière
	glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

	glm::vec3 eye = center - lightDir * (radius * 2.0f);
	glm::mat4 view = glm::lookAt(eye, center, up);
	glm::mat4 proj = glm::ortho(-radius, radius, -radius, radius, 0.0f, radius * kDepthRangeInRadii);

	return proj * view;
}

// --- Test d'ombre (appelé par le pixel shader) ---

static float ShadowVisibility(const Varyings& in, const Uniforms& u, float cosTheta)
{
	const RenderTarget* map = u.shadowMap;
	if (!map) return 1.0f;

	// MÊME transformation viewport que Pipeline::DrawTriangle (y inversé)
	float sx = (in.lightPos.x * 0.5f + 0.5f) * (float)map->width;
	float sy = (1.0f - (in.lightPos.y * 0.5f + 0.5f)) * (float)map->height;

	int x0 = (int)std::floor(sx);
	int y0 = (int)std::floor(sy);
	if (x0 < 0 || x0 >= map->width || y0 < 0 || y0 >= map->height)
		return 1.0f;

	float c = glm::max(cosTheta, 0.05f);
	float tanTheta = glm::min(std::sqrt(1.0f - c * c) / c, 10.0f);
	float bias = u.shadowBiasConst + u.shadowBiasSlope * tanTheta;

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

// --- Shaders ---

static Varyings DepthVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.mvp * glm::vec4(v.position, 1.0f);
	return out;
}

const Shader DepthShader = { DepthVS, nullptr };

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

// --- Classe ShadowMap ---

void ShadowMap::Init(int resolution)
{
	m_target = RenderTarget(resolution, resolution);
	m_dirty = true;
	RecomputeBias();
}

void ShadowMap::SetLight(const glm::vec3& direction, const AABB& sceneBounds)
{
	m_lightDir = glm::normalize(direction);
	m_radius   = glm::length(sceneBounds.max - sceneBounds.min) * 0.5f;
	m_lightVP  = ComputeLightVP(m_lightDir, sceneBounds, m_radius);
	RecomputeBias();
	m_dirty = true;
}

void ShadowMap::RecomputeBias()
{
	if (m_target.width <= 0) { m_biasUnit = 0.0f; return; }

	float texelWorld = (2.0f * m_radius) / (float)m_target.width;
	float depthRange = kDepthRangeInRadii * m_radius;
	m_biasUnit = 2.0f * texelWorld / depthRange;
}

bool ShadowMap::Update(Pipeline& pipeline, const std::vector<ShadowCaster>& casters, bool force)
{
	if (!m_dirty && !force)
		return false;

	Uniforms lightUniforms;
	lightUniforms.mvp = m_lightVP;

	pipeline.BindTarget(&m_target);
	pipeline.ClearDepth();
	pipeline.SetCullMode(CullMode::Front);
	pipeline.BindShader(DepthShader);
	pipeline.BindUniforms(lightUniforms);

	Frustum lightFrustum = Frustum::FromMatrix(m_lightVP);
	for (const ShadowCaster& caster : casters)
	{
		if (!caster.vertices || !lightFrustum.Intersects(caster.bounds))
			continue;

		pipeline.BindVertexBuffer(caster.vertices);
		pipeline.Draw();
	}

	m_dirty = false;
	return true;
}

void ShadowMap::ApplyTo(Uniforms& uniforms) const
{
	uniforms.lightDirection  = m_lightDir;
	uniforms.lightVP         = m_lightVP;
	uniforms.shadowMap       = &m_target;
	uniforms.shadowBiasConst = m_biasUnit * m_biasConstScale;
	uniforms.shadowBiasSlope = m_biasUnit * m_biasSlopeScale;
}

void ShadowMap::DrawDebug(FinalFrameBuffer& fb, int size) const
{
	for (int y = 0; y < size; ++y)
		for (int x = 0; x < size; ++x)
		{
			int sx = x * m_target.width  / size;
			int sy = y * m_target.height / size;
			float d = m_target.depth[(size_t)sy * m_target.width + sx];
			int g = (int)glm::clamp((d * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
			fb.put_pixel(x, y, (g << 16) | (g << 8) | g);
		}
}

#endif