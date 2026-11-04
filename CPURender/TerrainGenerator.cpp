#include "TerrainGenerator.h"
#include <iostream>
#include "Shaders.h"

TerrainGenerator::TerrainGenerator()
{
	m_Material.shader = &LambertShader;

	m_Material.specular = 0.01f;
	m_Material.roughness = 0.3f;
}

TerrainGenerator::~TerrainGenerator()
{
	if (m_Generation != nullptr) delete m_Generation;
}

void TerrainGenerator::GenerateHeighmap(glm::vec2 size, HeighMapType type, float spaceValue, double factorValue, float scale)
{
	if (m_Generation != nullptr) delete m_Generation;

	m_SpaceValue = spaceValue;
	m_FactorValue = factorValue;
	m_Scale = scale;

	switch (type)
	{
	case HeighMapType::_DIAMOND_SQUARE:
		m_Generation = Generation_Algorithm::DiamondSquare_HeighMap((size_t)size.x);
		m_FactorValue *= 0.025f;
		m_SpaceValue *= 5.f;
		m_Scale *= 0.1f;
		break;
	case HeighMapType::_PERLIN_2D:
		m_Generation = Generation_Algorithm::Perlin(size, 0.1);
		m_FactorValue *= 5.f;
		m_SpaceValue *= 0.25f;
		break;
	case HeighMapType::_OCTAVE_PERLIN_2D:
		m_Generation = Generation_Algorithm::PerlinOctave(size, 0.025, 5, 0.5);
		m_FactorValue *= 20.f;
		m_SpaceValue *= 0.25f;
		break;
	default:
		break;
	}

	if (m_Generation == nullptr) exit(-1);

	glm::dvec3 valueFactor = glm::dvec3{ m_SpaceValue, m_FactorValue, m_SpaceValue } * static_cast<double>(m_Scale);
	Generation_Algorithm::MeshTools::GenerateMesh(m_Mesh, *m_Generation, valueFactor, { ColorOf, NormalOf });
}

glm::dvec3 TerrainGenerator::NormalOf(Generation* pGeneration, glm::ivec2 heighmapID, glm::dvec3 valueFactor)
{
	std::vector<std::vector<double>>& rHeight = pGeneration->height;
	if (rHeight.size() <= 0) return glm::dvec3(1000.0);

	glm::dvec3 xDiff = { 0, 0, 0 };
	if (heighmapID.x - 1 >= 0)
		xDiff = glm::dvec3{ (heighmapID.x - 1), rHeight[heighmapID.y][heighmapID.x - 1],	heighmapID.y } * valueFactor;
	else
		xDiff = glm::dvec3{ heighmapID.x,		rHeight[heighmapID.y][heighmapID.x],		heighmapID.y } * valueFactor;
	if (heighmapID.x + 1 < rHeight[0].size())
		xDiff -= glm::dvec3{ (heighmapID.x + 1), rHeight[heighmapID.y][heighmapID.x + 1],	heighmapID.y } * valueFactor;
	else
		xDiff -= glm::dvec3{ heighmapID.x,		rHeight[heighmapID.y][heighmapID.x],		heighmapID.y } * valueFactor;

	glm::dvec3 zDiff = { 0, 0, 0 };
	if (heighmapID.y - 1 >= 0) // z - 1
		zDiff = glm::dvec3{ heighmapID.x,	rHeight[heighmapID.y - 1][heighmapID.x],	(heighmapID.y - 1) } * valueFactor;
	else
		zDiff = glm::dvec3{ heighmapID.x,	rHeight[heighmapID.y][heighmapID.x],		heighmapID.y } * valueFactor;
	if (heighmapID.y + 1 < rHeight.size()) // z + 1
		zDiff -= glm::dvec3{ heighmapID.x,	rHeight[heighmapID.y + 1][heighmapID.x],	(heighmapID.y + 1) } * valueFactor;
	else
		zDiff -= glm::dvec3{ heighmapID.x,	rHeight[heighmapID.y][heighmapID.x],		heighmapID.y } * valueFactor;

	glm::dvec3 cross = glm::cross(zDiff, xDiff);
	glm::dvec3 norm = glm::normalize(cross);
	return norm;
}

glm::vec4 TerrainGenerator::ColorOf(Generation* pGeneration, glm::ivec2 heighmapID)
{
	double value = (pGeneration->height[heighmapID.y][heighmapID.x] - pGeneration->min) / (pGeneration->max - pGeneration->min);
	glm::vec4 color{ 0, 0, 0, 0 };

	if (value > 0.6)			// SNOW
		color = glm::mix(ToColor(0x7F8386), ToColor(0xFFFFFF), std::clamp(value, 0.0, 1.0));
	else if (value > 0.55)	// STONE
		color = glm::mix(ToColor(0x3F9B0B), ToColor(0x7F8386), std::clamp(value, 0.0, 1.0));
	else																// SAND
		color = glm::mix(ToColor(0xC2B280), ToColor(0x3F9B0B), std::clamp(value, 0.0, 1.0));

	color.a = 255;
	return color;
}