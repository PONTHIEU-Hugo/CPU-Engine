#pragma once
#include "core.h"
#include "ProceduralGeneration.h"

enum class HeighMapType : uint8_t
{
	_DIAMOND_SQUARE
};

constexpr float HeighmapValueSpace = 2.5f;
constexpr float HeighmapValueFactor = 0.025f;

class TerrainGenerator
{
private:
	std::vector<Vertex> m_Vertices;
	Generation* m_Generation = nullptr;

public:
	TerrainGenerator() = default;
	~TerrainGenerator();

	std::vector<Vertex>* GetVertices() { return &m_Vertices; }

	void GenerateHeighmap(size_t size, HeighMapType type = HeighMapType::_DIAMOND_SQUARE);
};