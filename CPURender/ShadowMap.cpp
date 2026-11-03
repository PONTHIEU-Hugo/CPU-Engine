#include "ShadowMap.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Frustum.h"
#include "Shaders.h"  

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

void ShadowMap::Init(int resolution)
{
	m_target     = RenderTarget(resolution, resolution);
	m_tintTarget = RenderTarget(resolution, resolution);
	m_tintTarget.EnableTint();
	m_dirty = true;
	RecomputeBias();
}

void ShadowMap::SetLight(const glm::vec3& direction, const AABB& sceneBounds)
{
	m_lightDir = glm::normalize(direction);
	m_radius   = glm::length(sceneBounds.max - sceneBounds.min) * 0.5f;
	m_lightVP  = ComputeLightVP(m_lightDir, sceneBounds, m_radius);
	m_depthRange = kDepthRangeInRadii * m_radius;
	m_follow = false;
	RecomputeBias();
	m_dirty = true;
}

void ShadowMap::SetFollowMode(const glm::vec3& direction, float radius, float casterExtra)
{
	m_follow      = true;
	m_lightDir    = glm::normalize(direction);
	m_radius      = radius;
	m_casterExtra = casterExtra;
	m_depthRange  = 2.0f * radius + casterExtra;

	glm::vec3 up = (glm::abs(m_lightDir.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
	m_lightRot = glm::lookAt(glm::vec3(0.0f), m_lightDir, up);

	m_snappedCenter = glm::vec3(1e30f);
	RecomputeBias();
	m_dirty = true;
}

void ShadowMap::Follow(const glm::vec3& focus)
{
	if (m_follow == false) 
		return;

	float texel = (2.0f * m_radius) / (float)m_target.width;

	glm::vec3 c = glm::vec3(m_lightRot * glm::vec4(focus, 1.0f));
	c = glm::floor(c / texel + 0.5f) * texel;

	if (c == m_snappedCenter)
		return;

	m_snappedCenter = c;

	float d = -c.z;
	glm::mat4 proj = glm::ortho(c.x - m_radius, c.x + m_radius,
								c.y - m_radius, c.y + m_radius,
								d - m_radius - m_casterExtra,
								d + m_radius);
	m_lightVP = proj * m_lightRot;
	m_dirty = true;
}

void ShadowMap::RecomputeBias()
{
	if (m_target.width <= 0) { m_biasUnit = 0.0f; return; }

	float texelWorld = (2.0f * m_radius) / (float)m_target.width;
	m_biasUnit = 2.0f * texelWorld / m_depthRange;
}

bool ShadowMap::Update(Pipeline& pipeline, const std::vector<ShadowCaster>& opaques, const std::vector<ShadowCaster>& transparents, bool force)
{
	if (m_dirty == false && force == false)
		return false;

	Frustum lightFrustum = Frustum::FromMatrix(m_lightVP);

	Uniforms lightUniforms;
	lightUniforms.vp = m_lightVP;

	pipeline.BindTarget(&m_target);
	pipeline.ClearDepth();
	pipeline.SetCullMode(CullMode::Front);
	pipeline.BindShader(DepthShader);
	pipeline.BindUniforms(lightUniforms);

	for (const ShadowCaster& caster : opaques)
	{
		if (!caster.mesh || lightFrustum.Intersects(caster.bounds) == false)
			continue;
		pipeline.BindVertexBuffer(caster.mesh);
		pipeline.Draw(caster.world);
	}

	m_tintTarget.depth = m_target.depth;
	m_tintTarget.ClearTint(); 

	pipeline.BindTarget(&m_tintTarget);
	pipeline.SetCullMode(CullMode::Front);
	pipeline.SetBlend(false);
	pipeline.SetDepthWrite(true);
	pipeline.SetAlphaCutoff(0.0f);
	pipeline.BindShader(TintShader);

	for (const ShadowCaster& caster : transparents)
	{
		if (!caster.mesh || lightFrustum.Intersects(caster.bounds) == false)
			continue;

		Uniforms tintUniforms;
		tintUniforms.vp        = m_lightVP;
		tintUniforms.baseColor = caster.tint;

		pipeline.BindUniforms(tintUniforms);
		pipeline.BindVertexBuffer(caster.mesh);
		pipeline.Draw(caster.world);
	}

	m_dirty = false;
	return true;
}

void ShadowMap::ApplyTo(Uniforms& uniforms) const
{
	uniforms.lightDirection  = m_lightDir;
	uniforms.lightVP         = m_lightVP;
	uniforms.shadowMap       = &m_target;
	uniforms.shadowTint		 = &m_tintTarget;
	uniforms.shadowBiasConst = m_biasUnit * m_biasConstScale;
	uniforms.shadowBiasSlope = m_biasUnit * m_biasSlopeScale;
}

void ShadowMap::DrawDebug(FinalFrameBuffer& fb, int size) const
{
	if (m_target.width <= 0 || m_target.height <= 0)
		return;

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
