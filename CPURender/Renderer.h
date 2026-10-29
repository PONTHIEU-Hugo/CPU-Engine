#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "Pipeline.h"
#include "ShadowMap.h"

struct Scene;
struct RenderObject;
struct CubeMap;

class Renderer
{
public:
	void Init(int shadowResolution);
	void Render(const Scene& scene, FinalFrameBuffer& out);

	bool showShadowDebug = true;
	int  shadowDebugSize = 64;

private:
	void UpdateLight(const Scene& scene);
	bool ShadowCastersChanged(const Scene& scene);
	void ShadowPass(const Scene& scene);
	void MainPass(const Scene& scene, FinalFrameBuffer& out);
	void DrawObject(const RenderObject& object, const Uniforms& base);
	void ParticlePass(const Scene& scene, const Uniforms& base);
	void DrawSkybox(const CubeMap& skybox, const glm::mat4& view, const glm::mat4 proj, FinalFrameBuffer& out);

	Pipeline  m_pipeline;
	ShadowMap m_shadow;

	// Etat de la lumiere au dernier SetFollowMode (pour detecter un changement).
	glm::vec3 m_lastLightDir = glm::vec3(0.0f);
	float     m_lastShadowRadius = -1.0f;
	float     m_lastCasterExtra = -1.0f;

	// Signature (mesh + version du Transform) des objets qui projettent une ombre.
	uint64_t  m_casterSignature = 0;

	std::vector<ShadowCaster> m_casters;   // reutilise d'une frame a l'autre
};

#endif // !RENDERER_H_INCLUDED