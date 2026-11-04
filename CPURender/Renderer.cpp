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
	SkinObjects(scene);
	ShadowPass(scene);
	MainPass(scene, out);
	TransparencyPass(scene, out);

	if (showShadowDebug)
		m_shadow.DrawDebug(out, shadowDebugSize);
}

void Renderer::UpdateLight(const Scene& scene)
{
	const DirectionalLight& sun = scene.sun;

	if (sun.direction != m_lastLightDir
		|| sun.shadowRadius != m_lastShadowRadius
		|| sun.shadowCasterExtra != m_lastCasterExtra
		|| sun.color != m_lastColor)
	{
		m_shadow.SetFollowMode(sun.direction, sun.shadowRadius, sun.shadowCasterExtra, sun.color);
		m_lastLightDir     = sun.direction;
		m_lastShadowRadius = sun.shadowRadius;
		m_lastCasterExtra  = sun.shadowCasterExtra;
		m_lastColor        = sun.color;
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
		mix(object.transform->Version());

		if (object.material)
		{
			mix(object.material->transparent ? 1 : 0);
			mix((uint64_t)(object.material->baseColor.r * 4.0f));
			mix((uint64_t)(object.material->baseColor.g * 4.0f));
			mix((uint64_t)(object.material->baseColor.b * 4.0f));
			mix((uint64_t)(object.material->baseColor.a * 4.0f));
		}
		
		mix(object.transform->Version());
		if (object.animator)
			mix(object.animator->Version());
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
		caster.mesh   = GetMeshToDraw(object);
		caster.world  = object.transform->GetWorldMatrix();
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

void Renderer::SkinVertex(const Mesh& mesh, size_t i,
					   const std::vector<glm::mat4>& bones,
					   glm::vec3& outPos, glm::vec3& outNormal, glm::vec4& outTangent)
{
	const Vertex& v = mesh.vertices[i];
	const VertexSkin& s = mesh.skin[i];

	glm::mat4 skin(0.0f);
	float total = 0.0f;
	for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
	{
		if (s.boneIDs[k] < 0) continue;
		skin += bones[s.boneIDs[k]] * s.weights[k];
		total += s.weights[k];
	}
	if (total <= 0.0f) skin = glm::mat4(1.0f);

	outPos    = glm::vec3(skin * glm::vec4(v.position, 1.0f));
	outNormal  = glm::normalize(glm::mat3(skin) * v.normal);
	outTangent = glm::vec4(glm::normalize(glm::mat3(skin) * glm::vec3(v.tangent)), v.tangent.w);
}

void Renderer::SkinObjects(const Scene& scene)
{
	for (const RenderObject& object : scene.objects)
	{
		if (!object.mesh || !object.animator || !object.mesh->IsSkinned())
			continue;

		const auto& bones = object.animator->GetFinalBoneMatrices();
		Mesh& dst = m_skinnedMeshes[&object];
		const Mesh& src = *object.mesh;

		dst.vertices = src.vertices;
		for (size_t i = 0; i < src.vertices.size(); ++i)
			SkinVertex(src, i, bones, dst.vertices[i].position, dst.vertices[i].normal, dst.vertices[i].tangent);
		dst.ComputeBounds();
	}
}

const Mesh* Renderer::GetMeshToDraw(const RenderObject& object) const
{
	auto it = m_skinnedMeshes.find(&object);
	return it != m_skinnedMeshes.end() ? &it->second : object.mesh;
}

void Renderer::MainPass(const Scene& scene, FinalFrameBuffer& out)
{
	out.clear(0x000000);
	m_pipeline.BindFrameBuffer(&out);
	m_pipeline.ClearDepth();

	const glm::mat4& view = scene.camera->GetViewMatrix();
	const glm::mat4& proj = scene.camera->GetProjMatrix();

	const glm::mat4& viewProj = scene.camera->GetVPMatrix();

	Uniforms base;
	BuildBaseUniforms(scene, base);

	Frustum frustum = Frustum::FromMatrix(viewProj);

	m_pipeline.SetAlphaCutoff(0.0f);
	
	
	const RenderObject* skybox = nullptr;

	for (const RenderObject& object : scene.objects)
	{
		if (!object.visible || !object.mesh || !object.material || !object.material->shader || object.material->transparent)
			continue;

		if (object.material->isSkybox)
		{
			if (!skybox)
			{
				skybox = &object;
				continue;
			}
		}

		if (frustum.Intersects(object.WorldBounds()) == false)
			continue;
		
		const bool skinned = object.mesh->IsSkinned() && object.animator;
		const std::vector<glm::mat4>* bones = skinned ? &object.animator->GetFinalBoneMatrices() : nullptr;

		if (skinned)
		{
			const Mesh& src = *object.mesh;
			m_skinnedMesh.vertices = src.vertices; 
			for (size_t i = 0; i < src.vertices.size(); ++i)
				SkinVertex(src, i, *bones,
						   m_skinnedMesh.vertices[i].position,
						   m_skinnedMesh.vertices[i].normal,
						   m_skinnedMesh.vertices[i].tangent);
		}
		DrawObject(object, base, GetMeshToDraw(object));
	}
	DrawSkybox(skybox, base);
	ParticlePass(scene, base);
}

// Alpha minimum (0-255) pour qu'un pixel d'une particule opaque soit dessine.
static const float OPAQUE_ALPHA_CUTOFF = 128.0f;

void Renderer::ParticlePass(const Scene& scene, const Uniforms& base)
{
	const glm::mat4& view = scene.camera->GetViewMatrix();

	Uniforms uniforms = base;
	uniforms.texture = nullptr;
	uniforms.baseColor = glm::vec4(255.0f);
	uniforms.specularStrength = 0.0f;

	m_pipeline.SetLiteVaryings(true);
	m_pipeline.SetCullMode(CullMode::None);
	m_pipeline.BindShader(ParticleShader);
	m_pipeline.BindUniforms(uniforms);

	// Les emetteurs opaques d'abord (ils ecrivent la profondeur), puis les emetteurs avec melange par-dessus.
	for (int pass = 0; pass < 2; ++pass)
	{
		const bool opaquePass = (pass == 0);

		if (opaquePass)
		{
			// Pas de melange : un pixel est soit entierement dessine, soit rejete (alpha sous le seuil -> bords en disque).
			m_pipeline.SetBlend(false);
			m_pipeline.SetAdditive(false);
			m_pipeline.SetDepthWrite(true);
			m_pipeline.SetAlphaCutoff(OPAQUE_ALPHA_CUTOFF);
		}
		else
		{
			m_pipeline.SetBlend(true);
			m_pipeline.SetDepthWrite(false);
			m_pipeline.SetAlphaCutoff(0.0f);
		}

		for (Emitter* emitter : scene.emitters)
		{
			if (emitter == nullptr || emitter->settings.opaque != opaquePass)
				continue;

			emitter->BuildQuads(view);
			if (emitter->GetMesh().vertices.empty())
				continue;

			if (opaquePass == false)
				m_pipeline.SetAdditive(emitter->GetBlendMode() == BlendMode::Additive);
			m_pipeline.BindVertexBuffer(&emitter->GetMesh());
			m_pipeline.Draw(glm::mat4(1.0f));
		}
	}

	m_pipeline.SetAdditive(false);
	m_pipeline.SetLiteVaryings(false);
	m_pipeline.SetBlend(false);
	m_pipeline.SetDepthWrite(true);
}

void Renderer::DrawSkybox(const RenderObject* skybox, const Uniforms& base)
{
	if (!skybox) return;

	const Material& material = *skybox->material;
	if (!material.cubeMap || !material.cubeMap->enabled) return;

	Uniforms uniforms = base;
	uniforms.cubeMap = material.cubeMap;
	uniforms.texture = nullptr;   
	uniforms.normalMap = nullptr;

	m_pipeline.SetDepthWrite(false);
	m_pipeline.SetBlend(false);
	m_pipeline.SetLiteVaryings(false);  
	m_pipeline.SetCullMode(material.cull);
	m_pipeline.BindShader(*material.shader);
	m_pipeline.BindVertexBuffer(skybox->mesh);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw(skybox->transform->GetWorldMatrix());

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
	
	if (m_transparents.empty())
		return;
	
	std::sort(m_transparents.begin(), m_transparents.end(),
	[](const TransparentEntry& a, const TransparentEntry& b) { return a.dist2 > b.dist2; });
	
	std::memcpy(m_sceneColor->colorData, out.colorData, sizeof(out.colorData));
	base.sceneColor = m_sceneColor.get();
	base.sceneDepth = &m_pipeline.GetScreenDepth();

	m_pipeline.SetBlend(true);
	m_pipeline.SetDepthWrite(false);
	for (const TransparentEntry& entry : m_transparents)
		DrawObject(*entry.object, base, GetMeshToDraw(*entry.object));
	m_pipeline.SetBlend(false);
	m_pipeline.SetDepthWrite(true);
}

void Renderer::DrawObject(const RenderObject& object, const Uniforms& base, const Mesh* meshOverride)
{
	const Material& material = *object.material;

	Uniforms uniforms				= base;
	uniforms.texture				= material.texture;
	uniforms.normalMap				= material.normalMap;
	uniforms.normalStrength			= material.normalStrength;
	uniforms.normalFlipY			= material.normalFlipY;
	uniforms.baseColor				= material.baseColor;
	uniforms.armMap					= material.armMap;
	uniforms.aoMap					= material.aoMap;
	uniforms.roughnessMap			= material.roughnessMap;
	uniforms.metalnessMap			= material.metalnessMap;
	uniforms.eta					= material.eta;
	uniforms.thickness				= material.thickness;
	uniforms.specularStrength		= material.specular;
	uniforms.shininess				= ShininessFromRoughness(material.roughness);
	uniforms.displacementMap        = material.displacementMap;
	uniforms.displacementScale      = material.displacementScale;
	uniforms.displacementMidpoint   = material.displacementMidpoint;
	uniforms.displacementPixelEdge  = material.displacementPixelEdge;

	m_pipeline.SetCullMode(material.cull);
	m_pipeline.BindShader(*material.shader);
	m_pipeline.BindVertexBuffer(meshOverride ? meshOverride : object.mesh);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw(object.transform->GetWorldMatrix());
}
