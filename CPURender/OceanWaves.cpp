#include "OceanWaves.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"
#include "Chrono.h"

OceanWaves::OceanWaves()
{
	m_Material.shader = &SinWavesShader;

	m_Material.specular = 0.01f;
	m_Material.roughness = 0.3f;
	m_Material.amplitude = 5.f;

	GenerateMesh({ 15, 5 });
	SetWaveNumber(1);
	SetOrientation({ -0.5f, 0.5f });
}

void OceanWaves::SetWaveNumber(size_t waveNumber)
{
	m_WaveNumber = waveNumber;
	UpdateLambda();
}

void OceanWaves::SetOrientation(glm::vec2 newDir)
{
	m_Dir = glm::normalize(newDir);
	m_Material.direction = m_Dir;
	UpdateLambda();
}

void OceanWaves::SetAmplitude(float newAmplitude)
{
	m_Material.amplitude = newAmplitude;
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

void OceanWaves::UpdateLambda()
{
	float x = std::abs(m_Size.x * m_Dir.x);
	float y = std::abs(m_Size.y * m_Dir.y);
	m_Material.lambda = (x + y) / static_cast<float>(m_WaveNumber); // Distance entre les vagues
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
