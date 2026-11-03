#pragma once
#include "Mesh.h"
#include "Material.h"
#include "ProceduralGeneration.h"

enum class HeighMapType : uint8_t
{
	_DIAMOND_SQUARE,
	_PERLIN_2D,
	_OCTAVE_PERLIN_2D
};

constexpr float HeighmapValueSpace = 1.f;

class TerrainGenerator
{
private:
	Mesh m_Mesh;
	Material m_Material;
	Generation* m_Generation = nullptr;

public:
	TerrainGenerator();
	~TerrainGenerator();

	Mesh& GetMesh() { return m_Mesh; }
	const Material& GetMaterial() { return m_Material; }

	void GenerateHeighmap(size_t size, HeighMapType type, double scale = 1.0) { return GenerateHeighmap(glm::vec2{ size, size }, type, scale); }
	void GenerateHeighmap(glm::vec2 size, HeighMapType type, double scale = 1.0);

private:
	glm::dvec3 NormalOf(glm::ivec2 heighmapID, double heighmapValueFactor, double scale);
	glm::vec4 ColorOf(glm::ivec2 heighmapID, double heighmapValueFactor, double scale);
};