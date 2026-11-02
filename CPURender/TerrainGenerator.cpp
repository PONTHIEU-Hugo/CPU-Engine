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

void TerrainGenerator::GenerateHeighmap(glm::vec2 size, HeighMapType type, double scale)
{
	if (m_Generation != nullptr) delete m_Generation;

	double heighmapValueFactor = 1.0;
	switch (type)
	{
	case HeighMapType::_DIAMOND_SQUARE:
		m_Generation = Generation_Algorithm::DiamondSquare_HeighMap(size.x);
		heighmapValueFactor = 0.025;
		break;
	case HeighMapType::_PERLIN_2D:
		m_Generation = Generation_Algorithm::Perlin(size, 0.1);
		heighmapValueFactor = 5.0;
		break;
	case HeighMapType::_OCTAVE_PERLIN_2D:
		m_Generation = Generation_Algorithm::PerlinOctave(size, 0.1, 5, 0.5);
		heighmapValueFactor = 20.0;
		break;
	default:
		break;
	}

	if (m_Generation == nullptr) exit(-1);
	std::vector<std::vector<double>>& rHeight = m_Generation->height;

	glm::vec3 rSize = m_Generation->size;
	m_Mesh.vertices.clear();

	for (int y = 0; y < rHeight.size(); y++)
	{
		for (int x = 0; x < rHeight[y].size(); x++)
		{
			Vertex current{
				glm::vec3{x * HeighmapValueSpace * scale, static_cast<float>(rHeight[y][x] * heighmapValueFactor * scale), y * HeighmapValueSpace * scale},
				NormalOf({x, y}, heighmapValueFactor, scale),
				ColorOf({x, y}, heighmapValueFactor, scale),
				glm::vec2(0.f, 0.f)
			};

			if (x + 1 >= rHeight[0].size()) continue;
			if (y + 1 >= rHeight.size()) continue;

			Vertex corner{
				glm::vec3{(x + 1) * HeighmapValueSpace * scale, static_cast<float>(rHeight[y + 1][x + 1] * heighmapValueFactor * scale), (y + 1) * HeighmapValueSpace * scale},
				NormalOf({x + 1, y + 1}, heighmapValueFactor, scale),
				ColorOf({x + 1, y + 1}, heighmapValueFactor, scale),
				glm::vec2(0.f, 0.f)
			};

			m_Mesh.vertices.push_back(current);
			m_Mesh.vertices.push_back(Vertex{
				glm::vec3{x * HeighmapValueSpace * scale, static_cast<float>(rHeight[y + 1][x] * heighmapValueFactor * scale), (y + 1) * HeighmapValueSpace * scale},
				NormalOf({x, y + 1}, heighmapValueFactor, scale),
				ColorOf({x, y + 1}, heighmapValueFactor, scale),
			glm::vec2(0.f, 0.f)
				});
			m_Mesh.vertices.push_back(corner);

			m_Mesh.vertices.push_back(current);
			m_Mesh.vertices.push_back(corner);
			m_Mesh.vertices.push_back(Vertex{
				glm::vec3{(x + 1) * HeighmapValueSpace * scale, static_cast<float>(rHeight[y][x + 1] * heighmapValueFactor * scale), y * HeighmapValueSpace * scale},
				NormalOf({x + 1, y}, heighmapValueFactor, scale),
				ColorOf({x + 1, y}, heighmapValueFactor, scale),
				glm::vec2(0.f, 0.f)
				});
		}
	}
	m_Mesh.ComputeBounds();
}

glm::dvec3 TerrainGenerator::NormalOf(glm::ivec2 heighmapID, double heighmapValueFactor, double scale)
{
	std::vector<std::vector<double>>& rHeight = m_Generation->height;
	if (rHeight.size() <= 0) return glm::dvec3(1000.0);

	glm::dvec3 xDiff = { 0, 0, 0 };
	if (heighmapID.x - 1 >= 0) // x - 1
		xDiff = glm::dvec3{ (heighmapID.x - 1) * HeighmapValueSpace * scale, rHeight[heighmapID.y][heighmapID.x - 1] * heighmapValueFactor * scale, heighmapID.y * HeighmapValueSpace * scale };
	else
		xDiff = glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale, heighmapID.y * HeighmapValueSpace * scale };
	if (heighmapID.x + 1 < rHeight[0].size()) // x + 1
		xDiff -= glm::dvec3{(heighmapID.x + 1)* HeighmapValueSpace* scale, rHeight[heighmapID.y][heighmapID.x + 1] * heighmapValueFactor * scale, heighmapID.y * HeighmapValueSpace * scale };
	else
		xDiff -= glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale, heighmapID.y * HeighmapValueSpace * scale };

	glm::dvec3 zDiff = { 0, 0, 0 };
	if (heighmapID.y - 1 >= 0) // z - 1
		zDiff = glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, rHeight[heighmapID.y - 1][heighmapID.x] * heighmapValueFactor * scale, (heighmapID.y - 1) * HeighmapValueSpace * scale };
	else
		zDiff = glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale, heighmapID.y * HeighmapValueSpace * scale };
	if (heighmapID.y + 1 < rHeight.size()) // z + 1
		zDiff -= glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, rHeight[heighmapID.y + 1][heighmapID.x] * heighmapValueFactor * scale, (heighmapID.y + 1) * HeighmapValueSpace * scale };
	else
		zDiff -= glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale, heighmapID.y * HeighmapValueSpace * scale };
	
	glm::dvec3 cross = glm::cross(zDiff, xDiff);
	glm::dvec3 norm = glm::normalize(cross);
	return norm;
}

glm::vec4 TerrainGenerator::ColorOf(glm::ivec2 heighmapID, double heighmapValueFactor, double scale)
{
	glm::vec4 color;
	if (m_Generation->height[heighmapID.y][heighmapID.x] > 0.6)			// SNOW
		color = ToColor(0xFFFFFF);
	else if (m_Generation->height[heighmapID.y][heighmapID.x] > 0.55)	// STONE
		color = ToColor(0x7F8386);
	else if (m_Generation->height[heighmapID.y][heighmapID.x] > 0.35)	// GRASS
		color = ToColor(0x3F9B0B);
	else																// SAND
		color = ToColor(0xC2B280);

	color.a = 255;
	return color;
}
