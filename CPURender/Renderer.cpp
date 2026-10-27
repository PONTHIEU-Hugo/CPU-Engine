#include "Renderer.h"

#include "Scene.h"
#include "Frustum.h"

void Renderer::Init(int shadowResolution)
{
	m_shadow.Init(shadowResolution);
}

void Renderer::Render(const Scene& scene, FinalFrameBuffer& out)
{
	UpdateLight(scene);
	ShadowPass(scene);
	MainPass(scene, out);

	if (showShadowDebug)
		m_shadow.DrawDebug(out, shadowDebugSize);
}

void Renderer::UpdateLight(const Scene& scene)
{
	const DirectionalLight& sun = scene.sun;

	if (sun.direction != m_lastLightDir
		|| sun.shadowRadius != m_lastShadowRadius
		|| sun.shadowCasterExtra != m_lastCasterExtra)
	{
		m_shadow.SetFollowMode(sun.direction, sun.shadowRadius, sun.shadowCasterExtra);
		m_lastLightDir     = sun.direction;
		m_lastShadowRadius = sun.shadowRadius;
		m_lastCasterExtra  = sun.shadowCasterExtra;
	}

	m_shadow.Follow(sun.shadowFocus);
}

bool Renderer::ShadowCastersChanged(const Scene& scene)
{
	uint64_t hash = 1469598103934665603ull;   // FNV-1a
	auto mix = [&hash](uint64_t value)
	{
		hash ^= value;
		hash *= 1099511628211ull;
	};

	for (size_t i = 0; i < scene.objects.size(); ++i)
	{
		const RenderObject& object = scene.objects[i];
		if (object.castShadow == false || !object.mesh)
			continue;

		mix(i);
		mix(reinterpret_cast<uintptr_t>(object.mesh));
		mix(object.transform.Version());
	}

	bool changed = hash != m_casterSignature;
	m_casterSignature = hash;
	return changed;
}

void Renderer::ShadowPass(const Scene& scene)
{
	bool castersChanged = ShadowCastersChanged(scene);
	if (castersChanged == false && m_shadow.IsDirty() == false)
		return;

	m_casters.clear();
	for (const RenderObject& object : scene.objects)
	{
		if (object.castShadow == false || !object.mesh)
			continue;

		ShadowCaster caster;
		caster.mesh   = object.mesh;
		caster.world  = object.transform.GetWorldMatrix();
		caster.bounds = object.WorldBounds();
		m_casters.push_back(caster);
	}

	m_shadow.Update(m_pipeline, m_casters, true);
}

void Renderer::MainPass(const Scene& scene, FinalFrameBuffer& out)
{
	out.clear(0x000000);
	m_pipeline.BindFrameBuffer(&out);
	m_pipeline.ClearDepth();

	const glm::mat4& view = scene.camera->GetViewMatrix();
	const glm::mat4& proj = scene.camera->GetProjMatrix();

	DrawSkybox(scene.skybox, view, proj, out);

	const glm::mat4& viewProj = scene.camera->GetVPMatrix();

	Uniforms base;
	base.vp     = viewProj;
	base.ambient = scene.sun.ambient;
	base.cameraPos = scene.camera->GetPosition();
	m_shadow.ApplyTo(base);

	Frustum frustum = Frustum::FromMatrix(viewProj);

	for (const RenderObject& object : scene.objects)
	{
		if (!object.visible || !object.mesh || !object.material || !object.material->shader)
			continue;

		if (frustum.Intersects(object.WorldBounds()) == false)
			continue;

		DrawObject(object, base);
	}
}

void Renderer::DrawObject(const RenderObject& object, const Uniforms& base)
{
	const Material& material = *object.material;

	Uniforms uniforms = base;
	uniforms.texture  = material.texture;
	uniforms.baseColor = material.baseColor;
	uniforms.specularStrength = material.specular;
	uniforms.shininess = ShininessFromRoughness(material.roughness);

	m_pipeline.SetCullMode(material.cull);
	m_pipeline.BindShader(*material.shader);
	m_pipeline.BindVertexBuffer(object.mesh);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw(object.transform.GetWorldMatrix());
}

void Renderer::DrawSkybox(const CubeMap& skybox, const glm::mat4& view, const glm::mat4 proj, FinalFrameBuffer& out)
{
	glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
	glm::mat4 invVP = glm::inverse(proj * viewNoTranslation);

	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			// Centre du pixel en NDC (y vers le haut)
			float ndcX = ((x + 0.5f) / WIDTH) * 2.0f - 1.0f;
			float ndcY = 1.0f - ((y + 0.5f) / HEIGHT) * 2.0f;

			glm::vec4 p = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
			glm::vec3 dir = glm::vec3(p) / p.w;

			glm::vec3 color = skybox.Sample(dir);
			out.put_pixel(x, y, (int)color.r << 16 | (int)color.g << 8 | (int)color.b);
		}
	}
}
