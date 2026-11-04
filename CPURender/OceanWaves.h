#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "WaveData.h"

class OceanWaves
{
public:
	Mesh m_Mesh;
	Material m_Material;

private:
	glm::ivec2 m_Size;
	std::vector<WaveData> m_Waves;

	static constexpr float _gravity = 9.81f;

	float m_TotalAmplitude = 0.f;
public:
	OceanWaves();
	~OceanWaves() = default;

	void AddWave(glm::vec2 dir, float amplitude, float sharpness, float lambda);

	void GenerateMesh(glm::ivec2 size, float cellSize = 1.f);
};