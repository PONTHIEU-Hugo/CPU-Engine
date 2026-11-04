#include "OceanWaves.h"
#include "Shaders.h"
#include "GeometryFactory.hpp"
#include "Chrono.h"
#include "ProceduralGeneration.h"

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
	m_RawSharpness.push_back(std::clamp(sharpness, 0.f, 1.f));

	if (lambda == 0.f)
		throw std::runtime_error("Lambda wave can't be 0)");

	float k = glm::two_pi<float>() / lambda;

	m_Waves.push_back({
		k,
		std::sqrt(_gravity * k),
		amplitude,
		0.f,
		dir
		});
	m_Material.waveDatas = m_Waves.data();
	size_t newSize = m_Waves.size();
	m_Material.waveNumber = static_cast<size_t>(newSize);

	for (int i = 0; i < m_Waves.size(); i++)
	{
		WaveData& rWave = m_Waves[i];
		float kAmplitude = rWave.k * rWave.amplitude;
		rWave.sharpness = kAmplitude > 0.f ? m_RawSharpness[i] / (kAmplitude * newSize) : 0.f;
	}

	m_Mesh.localBounds.min -= glm::vec3(amplitude);
	m_Mesh.localBounds.max += glm::vec3(amplitude);
}

void OceanWaves::GenerateMesh(glm::ivec2 size)
{
	Generation* pGeneration = Generation_Algorithm::PerlinOctave(size, 0.025, 4, 0.5);
	Generation_Algorithm::MeshTools::GenerateMesh(m_Mesh, *pGeneration, { 0.5, 0.6, 0.5 }, {});
	delete pGeneration;
	m_Size = size;
}