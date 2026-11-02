#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

class OceanWaves
{
public:
	Mesh m_Mesh;
	Material m_Material;

private:
	size_t m_WaveNumber;
	glm::ivec2 m_Size;
	glm::vec2 m_Dir;

public:
	OceanWaves();
	~OceanWaves() = default;

	void SetWaveNumber(size_t waveNumber) { m_WaveNumber = waveNumber; }
	void SetOrientation(glm::vec2 newDir);

	void Update();

private:
	void GenerateMesh(glm::ivec2 size);

	void AdaptColor(Vertex& rVertex);
};