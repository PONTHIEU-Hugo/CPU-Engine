#include "TerrainGenerator.h"
#include <iostream>

TerrainGenerator::~TerrainGenerator()
{
	if (m_Generation != nullptr) delete m_Generation;
}

void TerrainGenerator::GenerateHeighmap(size_t size, HeighMapType type)
{
	if (m_Generation != nullptr) delete m_Generation;
	m_Vertices.clear();
	m_Generation = Generation_Algorithm::DiamondSquare_HeighMap(size);

	std::vector<std::vector<int>>& rHeight = m_Generation->height;

	glm::vec3 rSize = m_Generation->size;
	m_Vertices.clear();

	for (int y = 0; y < rHeight.size(); y++)
	{
		for (int x = 0; x < rHeight[y].size(); x++)
		{
			Vertex current{
				glm::vec3{x * HeighmapValueSpace, -rHeight[y][x] * HeighmapValueFactor, y * HeighmapValueSpace},
				ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
			};

			for (int i = -1; i < 2; i++)
			{
				if (i == 0) continue;
				if (x + i < 0 || x + i >= rHeight.size()) continue;
				if (y + 1 >= rHeight.size()) continue;

				Vertex corner{
					glm::vec3{(x + i) * HeighmapValueSpace, -rHeight[y + 1][x + i] * HeighmapValueFactor, (y + 1) * HeighmapValueSpace},
					ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
				};

				m_Vertices.push_back(current);
				m_Vertices.push_back(corner);
				m_Vertices.push_back(Vertex{
					glm::vec3{x * HeighmapValueSpace, -rHeight[y + 1][x] * HeighmapValueFactor, (y + 1) * HeighmapValueSpace},
					ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
					});
				m_Vertices.push_back(current);
				m_Vertices.push_back(Vertex{
					glm::vec3{(x + i) * HeighmapValueSpace, -rHeight[y][x + i] * HeighmapValueFactor, y * HeighmapValueSpace},
					ToColor(RANDOM::GetRandomInt(0, 0xFFFFFF))
					});
				m_Vertices.push_back(corner);
			}
		}
	}
}