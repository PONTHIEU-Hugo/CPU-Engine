#include "OceanWaves.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"
#include "Chrono.h"

OceanWaves::OceanWaves()
{
	m_Material.shader = &LambertShader;

	m_Material.specular = 0.01f;
	m_Material.roughness = 0.3f;

	GenerateMesh({ 10, 10 });
	m_WaveNumber = 1;
	m_Dir = { -0.5f, 0.5f };
	glm::normalize(m_Dir);
}

void OceanWaves::SetOrientation(glm::vec2 newDir)
{
	m_Dir = newDir;
	glm::normalize(m_Dir);
}

void OceanWaves::Update()
{
	std::vector<Vertex>& rVertices = m_Mesh.vertices;

	float fT = static_cast<float>(CHRONO->GetTotalTime());
	float lambda = m_Size.x / static_cast<float>(m_WaveNumber); // Distance entre les vagues
	float k = (2 * 3.14) / lambda;		// Nombre d'onde
	float w = 1 * fT;					// Waves pulsation

	for (Vertex& rVertex : rVertices)
	{
		glm::vec2 localPos = { rVertex.position.x - m_Mesh.localBounds.min.x, rVertex.position.z - m_Mesh.localBounds.min.z };
		float current = ((localPos.x * m_Dir.x + localPos.y * m_Dir.y) * 0.5f);

		rVertex.position.y = std::sinf(k * current - w);
		AdaptColor(rVertex);
	}
}

void OceanWaves::GenerateMesh(glm::ivec2 size)
{
	m_Mesh.vertices.clear();
	glm::vec4 color = ToColor(0x0041C2);
	color.a = 255;

	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			Vertex current{
				{x, 1, y},
				{0, 1, 0},
				color,
				{0.f, 0.f}
			};

			Vertex corner{
				{x + 1, 1, y + 1},
				{0, 1, 0},
				color,
				{0.f, 0.f}
			};

			m_Mesh.vertices.push_back(current);
			m_Mesh.vertices.push_back(Vertex{
				{x, 1, y + 1},
				{0, 1, 0},
				color,
				{0.f, 0.f}
			});
			m_Mesh.vertices.push_back(corner);

			m_Mesh.vertices.push_back(current);
			m_Mesh.vertices.push_back(corner);
			m_Mesh.vertices.push_back(Vertex{
				{x + 1, 1, y},
				{0, 1, 0},
				color,
				{0.f, 0.f}
			});
		}
	}
	m_Mesh.ComputeBounds();
	m_Size = size;
}

void OceanWaves::AdaptColor(Vertex& rVertex)
{
	// Blue colors

	glm::vec4 color;
	if (rVertex.position.y > 0.6)
		color = ToColor(0x5CB5E1);
	else if (rVertex.position.y > 0.55)
		color = ToColor(0x3EA4F0);
	else if (rVertex.position.y > 0.35)
		color = ToColor(0x5D97E7);
	else
		color = ToColor(0x416BDF);

	color.a = 255;
	rVertex.color = color;
}
