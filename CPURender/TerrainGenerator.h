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

class TerrainGenerator
{
private:
	Mesh m_Mesh;
	Material m_Material;
	Generation* m_Generation = nullptr;

	float m_Scale = 1.f;
	float m_SpaceValue = 1.f;
	double m_FactorValue = 1.0;
	
	glm::vec3 m_Offset{ 0.f };
	
public:
	TerrainGenerator();
	~TerrainGenerator();

	Mesh& GetMesh() { return m_Mesh; }
	const Material& GetMaterial() { return m_Material; }

	void GenerateHeighmap(size_t size, HeighMapType type, float spaceValue = 1.f, double factorValue = 1.0, float scale = 1.f, bool mountain = false)
	{
		return GenerateHeighmap(glm::vec2{ size, size }, type, spaceValue, factorValue, scale, mountain);
	}
	void GenerateHeighmap(glm::vec2 size, HeighMapType type, float spaceValue = 1.f, double factorValue = 1.0, float scale = 1.f, bool mountain = false);

	double sharpness = 1.5; 
	double coneWeight = 0.4;
private:
	Vertex MakeVertex(glm::ivec2 heighmapID);
	glm::dvec3 NormalOf(glm::ivec2 heighmapID);
	glm::vec4 ColorOf(glm::ivec2 heighmapID);
};