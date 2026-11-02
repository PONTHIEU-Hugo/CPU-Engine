#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "Pipeline.h"   

struct Texture;

struct Material
{
	const Shader* shader = nullptr;
	const Texture* texture = nullptr;
	const Texture* normalMap = nullptr;
	const Texture* amrMap = nullptr;
	const Texture* aoMap = nullptr;
	const Texture* metalnessMap = nullptr;
	const Texture* roughnessMap = nullptr;
	
	glm::vec4      baseColor = { 255.0f, 255.0f, 255.0f, 255.0f };  
	CullMode       cull = CullMode::Back;

	float specular  = 0.0f;
	float roughness = 0.5f;
	float eta		= 0.66f;
	float thickness = 1.0f;
	float normalStrength = 1.0f;   
	bool  normalFlipY    = false; 
	
	bool transparent = false;
};

inline float ShininessFromRoughness(float roughness)
{
	float r = glm::clamp(roughness, 0.05f, 1.0f);
	return glm::max(2.0f / (r * r) - 2.0f, 1.0f);
}

#endif // !MATERIAL_H_INCLUDED