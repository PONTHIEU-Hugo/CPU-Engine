#include "OceanWaves.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"
#include "Chrono.h"

OceanWaves::OceanWaves()
{
	m_Material.shader = &GerstnerWavesShader;

	//m_Material.transparent = true;
	//m_Material.baseColor.a = 128.f;

	m_Material.specular = 0.5f;
	m_Material.roughness = 0.3f;

	GenerateMesh({ 15, 5 });
}

void OceanWaves::AddWave(glm::vec2 dir, float amplitude, float sharpness, float lambda)
{
	dir = glm::normalize(dir);
	sharpness = std::clamp(sharpness, 0.f, 1.f);

	if (lambda == 0.f)
	{
		std::runtime_error("Lambda wave can't be 0)");
		exit(-1);
	}

	float k = glm::two_pi<float>() / lambda;

	m_Waves.push_back({
		k,
		std::sqrt(_gravity * k),
		amplitude,
		sharpness,
		dir
		});
	m_Material.waveDatas = m_Waves.data();
	m_Material.waveNumber = static_cast<size_t>(m_Waves.size());

	m_Mesh.localBounds.min -= glm::vec3(amplitude);
	m_Mesh.localBounds.max += glm::vec3(amplitude);
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