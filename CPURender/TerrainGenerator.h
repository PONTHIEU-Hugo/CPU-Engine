#pragma once
#include "core.h"
#include "ProceduralGeneration.h"

enum class HeighMapType : uint8_t
{
	_DIAMOND_SQUARE,
	_PERLIN_2D
};

constexpr float HeighmapValueSpace = 1.f;

class TerrainGenerator
{
private:
	std::vector<Vertex> m_Vertices;
	Generation* m_Generation = nullptr;

public:
	TerrainGenerator() = default;
	~TerrainGenerator();

	std::vector<Vertex>* GetVertices() { return &m_Vertices; }

	void GenerateHeighmap(size_t size, HeighMapType type, int scale = 1) { return GenerateHeighmap(glm::vec2{ size, size }, type, scale); }
	void GenerateHeighmap(glm::vec2 size, HeighMapType type, int scale = 1);
};