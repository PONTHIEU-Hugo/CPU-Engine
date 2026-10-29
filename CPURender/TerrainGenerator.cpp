#include "TerrainGenerator.h"
#include <iostream>

TerrainGenerator::~TerrainGenerator()
{
	if (m_Generation != nullptr) delete m_Generation;
}

void TerrainGenerator::GenerateHeighmap(glm::vec2 size, HeighMapType type, int scale)
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
		m_Generation = Generation_Algorithm::Perlin(size, 0.1f);
		heighmapValueFactor = 5.0;
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
			glm::vec4 color = ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF));
			color.a = 255.0f;
			Vertex current{
				glm::vec3{x * HeighmapValueSpace * scale, -rHeight[y][x] * heighmapValueFactor * scale, y * HeighmapValueSpace * scale},
				NormalOf({x, y}, heighmapValueFactor, scale),
				color,
				glm::vec2(0.f, 0.f)
			};

			for (int i = -1; i < 2; i++)
			{
				if (i == 0) continue;
				if (x + i < 0 || x + i >= rHeight[0].size()) continue;
				if (y + 1 >= rHeight.size()) continue;

				color = ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF));
				color.a = 255.0f;
				Vertex corner{
					glm::vec3{(x + i) * HeighmapValueSpace * scale, -rHeight[y + 1][x + i] * heighmapValueFactor * scale, (y + 1) * HeighmapValueSpace * scale},
					NormalOf({x + i, y + 1}, heighmapValueFactor, scale),
					color,
					glm::vec2(0.f, 0.f)
				};

				color = ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF));
				color.a = 255.0f;
				m_Mesh.vertices.push_back(current);
				m_Mesh.vertices.push_back(corner);
				m_Mesh.vertices.push_back(Vertex{
					glm::vec3{x * HeighmapValueSpace * scale, -rHeight[y + 1][x] * heighmapValueFactor * scale, (y + 1) * HeighmapValueSpace * scale},
					NormalOf({x, y + 1}, heighmapValueFactor, scale),
					color,
				glm::vec2(0.f, 0.f)
					});
				
				color = ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF));
				color.a = 255.0f;
				m_Mesh.vertices.push_back(current);
				m_Mesh.vertices.push_back(Vertex{
					glm::vec3{(x + i) * HeighmapValueSpace * scale, -rHeight[y][x + i] * heighmapValueFactor * scale, y * HeighmapValueSpace * scale},
					NormalOf({x + i, y}, heighmapValueFactor, scale),
					color,
					glm::vec2(0.f, 0.f)
					});
				m_Mesh.vertices.push_back(corner);
			}
		}
	}
	m_Mesh.ComputeBounds();
}

glm::dvec3 TerrainGenerator::NormalOf(glm::ivec2 heighmapID, double heighmapValueFactor, int scale)
{
	std::vector<std::vector<double>>& rHeight = m_Generation->height;

	glm::dvec3 xDiff = { 0, 0, 0 };
	if (heighmapID.x - 1 >= 0) // x - 1
		xDiff = glm::dvec3{ (heighmapID.x - 1) * HeighmapValueSpace * scale, heighmapID.y * HeighmapValueSpace * scale, -rHeight[heighmapID.y][heighmapID.x - 1] * heighmapValueFactor * scale };
	else
		xDiff = glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, heighmapID.y * HeighmapValueSpace * scale, -rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale };
	if (heighmapID.x + 1 < rHeight[0].size()) // x + 1
		xDiff -= glm::dvec3{(heighmapID.x + 1)* HeighmapValueSpace* scale, heighmapID.y* HeighmapValueSpace* scale, -rHeight[heighmapID.y][heighmapID.x + 1] * heighmapValueFactor * scale };
	else
		xDiff -= glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, heighmapID.y * HeighmapValueSpace * scale, -rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale };

	glm::dvec3 zDiff = { 0, 0, 0 };
	if (heighmapID.y - 1 >= 0) // z - 1
		zDiff = glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, (heighmapID.y - 1) * HeighmapValueSpace * scale, -rHeight[heighmapID.y - 1][heighmapID.x] * heighmapValueFactor * scale };
	else
		zDiff = glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, heighmapID.y * HeighmapValueSpace * scale, -rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale };
	if (heighmapID.y + 1 < rHeight.size()) // z + 1
		zDiff -= glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, (heighmapID.y + 1) * HeighmapValueSpace * scale, -rHeight[heighmapID.y + 1][heighmapID.x] * heighmapValueFactor * scale };
	else
		zDiff -= glm::dvec3{ heighmapID.x * HeighmapValueSpace * scale, heighmapID.y * HeighmapValueSpace * scale, -rHeight[heighmapID.y][heighmapID.x] * heighmapValueFactor * scale };
	
	glm::dvec3 cross = glm::cross(xDiff, zDiff);
	glm::dvec3 norm = glm::normalize(cross);

	return norm;
}
