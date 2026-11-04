#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "Pipeline.h"   

struct Texture;
struct CubeMap;

struct Material
{
	const Shader* shader = nullptr;
	const Texture* texture = nullptr;
	const Texture* normalMap = nullptr;
	const Texture* armMap = nullptr;
	const Texture* aoMap = nullptr;
	const Texture* metalnessMap = nullptr;
	const Texture* roughnessMap = nullptr;
	const CubeMap* cubeMap = nullptr;
	
	const Texture* displacementMap = nullptr;
	float displacementScale = 0.0f;      
	float displacementMidpoint = 0.5f; 
	float displacementPixelEdge = 32.0f;
	
	glm::vec4      baseColor = { 255.0f, 255.0f, 255.0f, 255.0f };  
	CullMode       cull = CullMode::Back;

	float specular  = 0.0f;
	float roughness = 0.5f;
	float eta		= 0.66f;
	float thickness = 1.0f;
	float normalStrength = 1.0f;   
	bool  normalFlipY    = false;
	bool  isSkybox = false;
	
	bool transparent = false;

	WaveData* waveDatas = nullptr;	// ptr on array of waves
	uint8_t waveNumber = 0;			// number of wave => number of element in waves array
};

inline float ShininessFromRoughness(float roughness)
{
	float r = glm::clamp(roughness, 0.05f, 1.0f);
	return glm::max(2.0f / (r * r) - 2.0f, 1.0f);
}

#endif // !MATERIAL_H_INCLUDED