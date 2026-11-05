#include "OceanWaves.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"
#include "Chrono.h"

OceanWaves::OceanWaves()
{
	m_Material.shader = &GerstnerWavesShader;

	m_Material.transparent = true;
	m_Material.baseColor.a = 200.f;

	m_Material.specular = 0.5f;
	m_Material.roughness = 0.3f;

	GenerateMesh({ 15, 5 });
}

void OceanWaves::AddWave(glm::vec2 dir, float amplitude, float sharpness, float lambda)
{
	dir = glm::normalize(dir);
	sharpness = std::clamp(sharpness, 0.f, 1.f);

	if (lambda == 0.f)
		throw std::runtime_error("Lambda wave can't be 0");

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

	m_TotalAmplitude += amplitude;
	m_Mesh.localBounds.min -= glm::vec3(amplitude);
	m_Mesh.localBounds.max += glm::vec3(amplitude);
}

void OceanWaves::GenerateMesh(glm::ivec2 size, float cellSize)
{
	m_Mesh.vertices.clear();
	glm::vec4 color = ToColor(0x0041C2);
	color.a = 255;

	const float halfX = size.x * 0.5f;
	const float halfZ = size.y * 0.5f;

	auto MakeVertex = [&](int x, int z)
	{
		return Vertex{
			{ (x - halfX) * cellSize, 0.f, (z - halfZ) * cellSize },
			{ 0.f, 1.f, 0.f },
			color,
			{ 0.f, 0.f }
		};
	};

	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			Vertex current = MakeVertex(x,     y);
			Vertex right   = MakeVertex(x + 1, y);
			Vertex down    = MakeVertex(x,     y + 1);
			Vertex corner  = MakeVertex(x + 1, y + 1);

			m_Mesh.vertices.push_back(current);
			m_Mesh.vertices.push_back(down);
			m_Mesh.vertices.push_back(corner);

			m_Mesh.vertices.push_back(current);
			m_Mesh.vertices.push_back(corner);
			m_Mesh.vertices.push_back(right);
		}
	}
	m_Mesh.ComputeBounds();
	m_Mesh.localBounds.min -= glm::vec3(m_TotalAmplitude);
	m_Mesh.localBounds.max += glm::vec3(m_TotalAmplitude);
	m_Size = size;
}