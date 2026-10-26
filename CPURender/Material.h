#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "Pipeline.h"   

struct Texture;

// Comment dessiner une surface : shader + ressources + etats du pipeline.
struct Material
{
	const Shader* shader = nullptr;
	const Texture* albedo = nullptr;   // nullptr = couleur des sommets
	glm::vec3      baseColor = { 255.0f, 255.0f, 255.0f };   // teinte multipliee a la couleur (0-255), blanc = aucune
	CullMode       cull = CullMode::Back;

	// Reflet speculaire (Phong) : specular = intensite du reflet (0 = mat),
	// roughness = rugosite entre 0 (miroir, reflet petit et net) et 1 (mat, reflet large et faible).
	float          specular = 0.0f;
	float          roughness = 0.5f;
};

// Exposant de Phong d'une roughness : 0.05 -> ~800 (reflet tres net), 0.5 -> 6, 1.0 -> 1 (tres etale).
inline float ShininessFromRoughness(float roughness)
{
	float r = glm::clamp(roughness, 0.05f, 1.0f);
	return glm::max(2.0f / (r * r) - 2.0f, 1.0f);
}

#endif // !MATERIAL_H_INCLUDED