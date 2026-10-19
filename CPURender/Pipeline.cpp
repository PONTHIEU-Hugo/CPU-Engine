#include "Pipeline.h"
#include <algorithm>
#include <cmath>

Pipeline::Pipeline() : m_depth(WIDTH * HEIGHT, 1.0f) {}

void Pipeline::ClearDepth()
{
	std::fill(m_depth.begin(), m_depth.end(), 1.0f);
}

void Pipeline::Draw()
{
	if (!m_vertices || !m_frameBuffer || !m_vertexShader || !m_pixelShader) return;

	std::vector<Varyings> varyings;
	
	for (const Vertex& v : *m_vertices)
	{
		varyings.push_back(m_vertexShader(v, m_uniforms));
	}

	for (size_t i = 0; i + 2 < varyings.size(); i += 3)
	{
		DrawTriangle(varyings[i], varyings[i + 1], varyings[i + 2]);
	}
}

static float EdgeFunction(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p)
{
	return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void Pipeline::DrawTriangle(const Varyings& a, const Varyings& b, const Varyings& c)
{
	if (a.position.w <= 0 || b.position.w <= 0 || c.position.w <= 0)
		return;

	glm::vec3 v0 = glm::vec3(a.position) / a.position.w;
	glm::vec3 v1 = glm::vec3(b.position) / b.position.w;
	glm::vec3 v2 = glm::vec3(c.position) / c.position.w;

	glm::vec3 pixel0 = glm::vec3((v0.x * 0.5f + 0.5f) * WIDTH, (1.0f - (v0.y * 0.5f + 0.5f)) * HEIGHT, v0.z);
	glm::vec3 pixel1 = glm::vec3((v1.x * 0.5f + 0.5f) * WIDTH, (1.0f - (v1.y * 0.5f + 0.5f)) * HEIGHT, v1.z);
	glm::vec3 pixel2 = glm::vec3((v2.x * 0.5f + 0.5f) * WIDTH, (1.0f - (v2.y * 0.5f + 0.5f)) * HEIGHT, v2.z);

	float area = EdgeFunction(pixel0, pixel1, pixel2);
	if (area == 0.0f) return;

	int minX = (int)glm::clamp(glm::min(glm::min(pixel0.x, pixel1.x), pixel2.x), 0.0f, (float)(WIDTH - 1));
	int maxX = (int)glm::clamp(glm::max(glm::max(pixel0.x, pixel1.x), pixel2.x), 0.0f, (float)(WIDTH - 1));
	int minY = (int)glm::clamp(glm::min(glm::min(pixel0.y, pixel1.y), pixel2.y), 0.0f, (float)(HEIGHT - 1));
	int maxY = (int)glm::clamp(glm::max(glm::max(pixel0.y, pixel1.y), pixel2.y), 0.0f, (float)(HEIGHT - 1));

	const float invArea = 1.0f / area;

	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			glm::vec2 p(x + 0.5f, y + 0.5f);

			float w0 = EdgeFunction(pixel1, pixel2, p) * invArea;
			float w1 = EdgeFunction(pixel2, pixel0, p) * invArea;
			float w2 = EdgeFunction(pixel0, pixel1, p) * invArea;

			if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
				continue;

			float z = w0 * pixel0.z + w1 * pixel1.z + w2 * pixel2.z;
			if (z >= m_depth[y * WIDTH + x])
				continue;

			m_depth[y * WIDTH + x] = z;

			Varyings frag;
			frag.position = glm::vec4(p.x, p.y, z, 1.0f);
			frag.color = w0 * a.color + w1 * b.color + w2 * c.color;

			glm::vec3 col = m_pixelShader(frag, m_uniforms);
			col = glm::clamp(col, 0.0f, 1.0f);

			m_frameBuffer->put_pixel(x, y, (int)(col.r * 255) << 16 | (int)(col.g * 255) << 8 | (int)(col.b * 255));
		}
	}
}
