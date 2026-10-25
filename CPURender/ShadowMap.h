#ifndef SHADOW_MAP_H_INCLUDED
#define SHADOW_MAP_H_INCLUDED

#include <vector>
#include <glm/glm.hpp>
#include "Pipeline.h"
#include "Frustum.h"

struct ShadowCaster
{
	Geometry* geometries = nullptr;
	AABB bounds;
};

struct Transform;

class ShadowMap
{
public:
	void Init(int resolution);
	void SetLight(const glm::vec3& direction, const AABB& sceneBounds);
	void Invalidate() { m_dirty = true; }
	bool Update(Pipeline& pipeline, const std::vector<ShadowCaster>& casters, bool force = false);
	void ApplyTo(Uniforms& uniforms) const;
	void SetBiasScale(float constant, float slope) { m_biasConstScale = constant; m_biasSlopeScale = slope; }
	
	void DrawDebug(FinalFrameBuffer& framebuffer, int size) const;

	const glm::vec3& LightDirection() const { return m_lightDir; }
	const glm::mat4& LightVP()        const { return m_lightVP; }
	const RenderTarget& Target()      const { return m_target; }

private:
	RenderTarget m_target;
	glm::vec3 m_lightDir{ 0.0f, -1.0f, 0.0f };
	glm::mat4 m_lightVP{ 1.0f };
	float m_radius = 1.0f;          // rayon de la sphère englobant la scène
	float m_biasUnit = 0.0f;        
	float m_biasConstScale = 1.0f;
	float m_biasSlopeScale = 1.0f;
	bool  m_dirty = true;

	void RecomputeBias();
};

extern const Shader DepthShader;
extern const Shader ShadowLambertShader;

#endif