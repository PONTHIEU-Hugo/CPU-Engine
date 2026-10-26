#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "Pipeline.h"   

struct Texture;

// Comment dessiner une surface : shader + ressources + etats du pipeline.
struct Material
{
	const Shader* shader = nullptr;
	const Texture* albedo = nullptr;   // nullptr = couleur des sommets
	CullMode       cull = CullMode::Back;
};

#endif // !MATERIAL_H_INCLUDED