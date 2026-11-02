#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"
#include "Pipeline.h"
#include "ShadowMap.h"

struct RenderObject;
struct CubeMap;

class Scene;

class Renderer
{
public:
	void Init(int shadowResolution);
	void Render(const Scene& scene, FinalFrameBuffer& out);

	bool showShadowDebug = true;
	int  shadowDebugSize = 64;
	
	std::unique_ptr<FinalFrameBuffer> m_sceneColor = std::make_unique<FinalFrameBuffer>();

private:
	struct TransparentEntry { const RenderObject* object; float dist2; };
	
	void UpdateLight(const Scene& scene);
	bool ShadowCastersChanged(const Scene& scene);
	void ShadowPass(const Scene& scene);
	void SkinVertex(const Mesh& mesh, size_t i, const std::vector<glm::mat4>& bones, glm::vec3& outPos, glm::vec3& outNormal, glm::vec4& outTangent);
	void SkinObjects(const Scene& scene);
	const Mesh* GetMeshToDraw(const RenderObject& object) const;
	void MainPass(const Scene& scene, FinalFrameBuffer& out);
	void BuildBaseUniforms(const Scene& scene, Uniforms& base);
	void TransparencyPass(const Scene& scene, FinalFrameBuffer& out);
	void DrawObject(const RenderObject& object, const Uniforms& base, const Mesh* meshOverride = nullptr);
	void ParticlePass(const Scene& scene, const Uniforms& base);
	void DrawSkybox(const CubeMap& skybox, const glm::mat4& view, const glm::mat4 proj, FinalFrameBuffer& out);

	Pipeline  m_pipeline;
	ShadowMap m_shadow;
	
	Mesh m_skinnedMesh;
	std::unordered_map<const RenderObject*, Mesh> m_skinnedMeshes;

	// Etat de la lumiere au dernier SetFollowMode (pour detecter un changement).
	glm::vec3 m_lastLightDir = glm::vec3(0.0f);
	float     m_lastShadowRadius = -1.0f;
	float     m_lastCasterExtra = -1.0f;

	// Signature (mesh + version du Transform) des objets qui projettent une ombre.
	uint64_t  m_casterSignature = 0;

	std::vector<TransparentEntry> m_transparents;
	std::vector<ShadowCaster> m_casters;
	std::vector<ShadowCaster> m_tintCasters;
};

#endif // !RENDERER_H_INCLUDED