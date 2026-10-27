#include "TerrainGenerator.h"
#include <iostream>

TerrainGenerator::~TerrainGenerator()
{
	if (m_Generation != nullptr) delete m_Generation;
}

void TerrainGenerator::GenerateHeighmap(glm::vec2 size, HeighMapType type, int scale)
{
	if (m_Generation != nullptr) delete m_Generation;
	m_Vertices.clear();
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
	m_Vertices.clear();

	for (int y = 0; y < rHeight.size(); y++)
	{
		for (int x = 0; x < rHeight[y].size(); x++)
		{
			Vertex current{
				glm::vec3{x * HeighmapValueSpace * scale, -rHeight[y][x] * heighmapValueFactor * scale, y * HeighmapValueSpace * scale},
				ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
			};

			for (int i = -1; i < 2; i++)
			{
				if (i == 0) continue;
				if (x + i < 0 || x + i >= rHeight.size()) continue;
				if (y + 1 >= rHeight.size()) continue;

				Vertex corner{
					glm::vec3{(x + i) * HeighmapValueSpace * scale, -rHeight[y + 1][x + i] * heighmapValueFactor * scale, (y + 1) * HeighmapValueSpace * scale},
					ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
				};

				m_Vertices.push_back(current);
				m_Vertices.push_back(corner);
				m_Vertices.push_back(Vertex{
					glm::vec3{x * HeighmapValueSpace * scale, -rHeight[y + 1][x] * heighmapValueFactor * scale, (y + 1) * HeighmapValueSpace * scale},
					ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
					});
				m_Vertices.push_back(current);
				m_Vertices.push_back(Vertex{
					glm::vec3{(x + i) * HeighmapValueSpace * scale, -rHeight[y][x + i] * heighmapValueFactor * scale, y * HeighmapValueSpace * scale},
					ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
					});
				m_Vertices.push_back(corner);
			}
		}
	}
}
