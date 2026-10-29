#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include <cstddef>
#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "Camera.h"
#include "CubeMap.h"
#include "ParticleSystem.h"

// Une instance dans la scene : quel mesh, avec quel materiau, ou.
struct RenderObject
{
	const Mesh* mesh = nullptr;
	const Material* material = nullptr;
	Transform       transform;
	bool            castShadow = true;
	bool            visible = true;

	AABB WorldBounds() const { return TransformAABB(mesh->localBounds, transform.GetWorldMatrix()); }
};

struct DirectionalLight
{
	glm::vec3 direction = { -0.5f, -1.0f, -0.3f };
	float     ambient = 0.2f;

	// Shadow map en mode "suivi" : un carre de cote 2*shadowRadius centre sur shadowFocus.
	float     shadowRadius = 50.0f;
	float     shadowCasterExtra = 30.0f;
	glm::vec3 shadowFocus = { 0.0f, 0.0f, 0.0f };
};

// Index stable tant qu'on ne retire pas d'objet.
using ObjectId = std::size_t;

struct Scene
{
	Camera*       camera = nullptr;
	DirectionalLight sun;
	CubeMap skybox;
	std::vector<RenderObject> objects;
	std::vector<Emitter*>     emitters;   // dessines apres les objets (voir Renderer::ParticlePass)

	// L'emetteur doit survivre a la scene (il est reference, pas copie).
	void AddEmitter(Emitter& emitter) { emitters.push_back(&emitter); }

	// Le mesh et le materiau doivent survivre a la scene (ils sont references, pas copies).
	ObjectId Add(const Mesh& mesh, const Material& material, bool castShadow = true, const Transform& transform = Transform())
	{
		RenderObject object;
		object.mesh = &mesh;
		object.material = &material;
		object.transform = transform;
		object.castShadow = castShadow;
		objects.push_back(object);
		return objects.size() - 1;
	}

	void LoadSkybox()
	{
		skybox.faces[0].Load("Assets/right.jpg");
		skybox.faces[1].Load("Assets/left.jpg");
		skybox.faces[2].Load("Assets/top.jpg");
		skybox.faces[3].Load("Assets/bottom.jpg");
		skybox.faces[4].Load("Assets/front.jpg");
		skybox.faces[5].Load("Assets/back.jpg");
	}

	RenderObject& Get(ObjectId id) { return objects[id]; }
	const RenderObject& Get(ObjectId id) const { return objects[id]; }
};

#endif // !SCENE_H_INCLUDED