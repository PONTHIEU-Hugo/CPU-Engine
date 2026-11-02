#ifndef SHADOW_MAP_H_INCLUDED
#define SHADOW_MAP_H_INCLUDED

#include <vector>
#include <glm/glm.hpp>
#include "Pipeline.h"
#include "AABB.h"

struct Mesh;

// Ce que la passe d'ombre doit dessiner (prepare par le Renderer).
struct ShadowCaster
{
	const Mesh* mesh = nullptr;
	glm::mat4   world{ 1.0f };
	AABB        bounds;
	glm::vec4   tint{ 255.0f };
};

// Shadow map d'une lumiere directionnelle. Simple membre du Renderer (plus de singleton).
class ShadowMap
{
public:
	void Init(int resolution);
	void SetLight(const glm::vec3& direction, const AABB& sceneBounds);
	void SetFollowMode(const glm::vec3& direction, float radius, float casterExtra);
	void Follow(const glm::vec3& focus);

	void Invalidate()     { m_dirty = true; }
	bool IsDirty() const  { return m_dirty; }

	bool Update(Pipeline& pipeline, const std::vector<ShadowCaster>& opaques, const std::vector<ShadowCaster>& transparents, bool force = false);
	void ApplyTo(Uniforms& uniforms) const;
	void SetBiasScale(float constant, float slope) { m_biasConstScale = constant; m_biasSlopeScale = slope; }

	void DrawDebug(FinalFrameBuffer& framebuffer, int size) const;

	const glm::vec3& LightDirection() const { return m_lightDir; }
	const glm::mat4& LightVP()        const { return m_lightVP; }
	const RenderTarget& Target()      const { return m_target; }

private:
	RenderTarget m_target;
	RenderTarget     m_tintTarget;

	glm::vec3 m_lightDir{ 0.0f, -1.0f, 0.0f };
	glm::mat4 m_lightVP{ 1.0f };
	float m_radius = 1.0f;

	bool m_follow = false;
	float m_casterExtra = 0.0f;
	float m_depthRange = 1.0f;
	glm::mat4 m_lightRot{ 1.0f };
	glm::vec3 m_snappedCenter{ 1e30f };

	float m_biasUnit = 0.0f;
	float m_biasConstScale = 1.0f;
	float m_biasSlopeScale = 1.0f;
	bool  m_dirty = true;

	void RecomputeBias();
};

#endif
