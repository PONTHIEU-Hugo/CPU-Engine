#include "Renderer.h"

#include "Scene.h"
#include "Frustum.h"
#include "Shaders.h"

void Renderer::Init(int shadowResolution)
{
	m_shadow.Init(shadowResolution);
}

void Renderer::Render(const Scene& scene, FinalFrameBuffer& out)
{
	UpdateLight(scene);
	ShadowPass(scene);
	MainPass(scene, out);
	std::memcpy(m_sceneColor.colorData, out.colorData, sizeof(out.colorData));
	m_sceneDepth = m_pipeline.GetScreenDepth();
	TransparencyPass(scene, out);

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
	uint64_t hash = 1469598103934665603ull;
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

		if (object.material)
		{
			mix(object.material->transparent ? 1 : 0);
			mix((uint64_t)(object.material->baseColor.r * 4.0f));
			mix((uint64_t)(object.material->baseColor.g * 4.0f));
			mix((uint64_t)(object.material->baseColor.b * 4.0f));
			mix((uint64_t)(object.material->baseColor.a * 4.0f));
		}
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
	m_tintCasters.clear();

	for (const RenderObject& object : scene.objects)
	{
		if (object.castShadow == false || !object.mesh)
			continue;

		ShadowCaster caster;
		caster.mesh   = object.mesh;
		caster.world  = object.transform.GetWorldMatrix();
		caster.bounds = object.WorldBounds();

		const bool transparent = object.material && object.material->transparent;
		if (transparent)
		{
			caster.tint = object.material->baseColor;
			m_tintCasters.push_back(caster);
		}
		else
			m_casters.push_back(caster);
	}

	m_shadow.Update(m_pipeline, m_casters, m_tintCasters, true);
}

void Renderer::MainPass(const Scene& scene, FinalFrameBuffer& out)
{
	out.clear(0x000000);
	m_pipeline.BindFrameBuffer(&out);
	m_pipeline.ClearDepth();

	const glm::mat4& view = scene.camera->GetViewMatrix();
	const glm::mat4& proj = scene.camera->GetProjMatrix();

	if (scene.skybox.enabled)
		DrawSkybox(scene.skybox, view, proj, out);

	const glm::mat4& viewProj = scene.camera->GetVPMatrix();

	Uniforms base;
	BuildBaseUniforms(scene, base);

	Frustum frustum = Frustum::FromMatrix(viewProj);

	m_pipeline.SetAlphaCutoff(0.0f);
	
	for (const RenderObject& object : scene.objects)
	{
		if (!object.visible || !object.mesh || !object.material || !object.material->shader || object.material->transparent)
			continue;

		if (frustum.Intersects(object.WorldBounds()) == false)
			continue;

		DrawObject(object, base);
	}

	ParticlePass(scene, base);
}

void Renderer::ParticlePass(const Scene& scene, const Uniforms& base)
{
	const glm::mat4& view = scene.camera->GetViewMatrix();

	Uniforms uniforms = base;
	uniforms.texture = nullptr;
	uniforms.baseColor = glm::vec4(255.0f);
	uniforms.specularStrength = 0.0f;

	m_pipeline.SetBlend(true);
	m_pipeline.SetDepthWrite(false);
	m_pipeline.SetAlphaCutoff(0.0f);
	m_pipeline.SetCullMode(CullMode::None);
	m_pipeline.BindShader(ParticleShader);
	m_pipeline.BindUniforms(uniforms);

	for (Emitter* emitter : scene.emitters)
	{
		if (emitter == nullptr)
			continue;

		emitter->BuildQuads(view);
		if (emitter->GetMesh().vertices.empty())
			continue;

		m_pipeline.BindVertexBuffer(&emitter->GetMesh());
		m_pipeline.Draw(glm::mat4(1.0f));
	}

	m_pipeline.SetBlend(false);
	m_pipeline.SetDepthWrite(true);
}

void Renderer::BuildBaseUniforms(const Scene& scene, Uniforms& base)
{
	base.vp        = scene.camera->GetVPMatrix();
	base.ambient   = scene.sun.ambient;
	base.cameraPos = scene.camera->GetPosition();
	m_shadow.ApplyTo(base);
}

void Renderer::TransparencyPass(const Scene& scene, FinalFrameBuffer& out)
{
	m_pipeline.BindFrameBuffer(&out);
	
	Uniforms base;
	BuildBaseUniforms(scene, base);

	base.sceneColor = &m_sceneColor;
	base.sceneDepth = &m_sceneDepth;
	
	Frustum frustum = Frustum::FromMatrix(base.vp);
	const glm::vec3 camPos = scene.camera->GetPosition();

	m_transparents.clear();
	for (const RenderObject& object : scene.objects)
	{
		if (!object.visible || !object.mesh || !object.material || !object.material->shader || !object.material->transparent)
			continue;

		AABB bounds = object.WorldBounds();
		if (frustum.Intersects(bounds) == false)
			continue;

		glm::vec3 centre = (bounds.min + bounds.max) * 0.5f;
		glm::vec3 d = centre - camPos;
		m_transparents.push_back({ &object, glm::dot(d, d) });
	}
	
	std::sort(m_transparents.begin(), m_transparents.end(),
	[](const TransparentEntry& a, const TransparentEntry& b) { return a.dist2 > b.dist2; });
	
	m_pipeline.SetBlend(true);
	m_pipeline.SetDepthWrite(false);
	
	for (const TransparentEntry& entry : m_transparents)
		DrawObject(*entry.object, base);

	m_pipeline.SetBlend(false);
	m_pipeline.SetDepthWrite(true);
}

void Renderer::DrawObject(const RenderObject& object, const Uniforms& base)
{
	const Material& material = *object.material;

	Uniforms uniforms		  = base;
	uniforms.texture		  = material.texture;
	uniforms.baseColor		  = material.baseColor;
	uniforms.eta			  = material.eta;
	uniforms.thickness		  = material.thickness;
	uniforms.specularStrength = material.specular;
	uniforms.shininess		  = ShininessFromRoughness(material.roughness);

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

	for (int y = 0; y < HEIGHT; ++y) 
	{
		for (int x = 0; x < WIDTH; ++x) 
		{
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
