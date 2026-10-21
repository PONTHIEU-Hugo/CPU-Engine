#include "TerrainGenerator.h"

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
	m_Vertices.resize(rSize.x * rSize.y * rSize.z);
	
	for (size_t y = 0; y < rHeight.size(); y++)
	{
		for (size_t x = 0; x < rHeight[y].size(); x++)
		{
			m_Vertices[rHeight.size() * y + x] = Vertex{ 
				glm::vec3{ x, -rHeight[x][y] * 0.01f, y },
				ToColor(0xFFFFFF) 
			};
		}
	}
}
