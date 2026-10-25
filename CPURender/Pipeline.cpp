#include "Pipeline.h"
#include "Clipping.h"

Pipeline::Pipeline() : m_screen(WIDTH, HEIGHT)
{
	m_target = &m_screen;
}

void Pipeline::ClearDepth()
{
	if (m_target) m_target->ClearDepth();
}

void Pipeline::Draw(glm::mat4 const& mat)
{
	if (!m_geometry || !m_target || !m_vertexShader) return;
	if (m_target->color && !m_pixelShader) return;

	const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat)));

	std::vector<Varyings> varyings;
	varyings.reserve(m_geometry->vertices.size());

	for (const Vertex& src : m_geometry->vertices)
	{
		Vertex v = src;
		v.position = glm::vec3(mat * glm::vec4(src.position, 1.0f));
		v.normal   = glm::normalize(normalMatrix * src.normal);

		varyings.push_back(m_vertexShader(v, m_uniforms));
	}

	for (size_t i = 0; i + 2 < varyings.size(); i += 3)
	{
		Varyings poly[12];
		int count = ClipTriangle(varyings[i], varyings[i + 1], varyings[i + 2], poly);

		for (int k = 1; k + 1 < count; ++k)
			DrawTriangle(poly[0], poly[k], poly[k + 1]);
	}
}

static float EdgeFunction(const glm::vec2& edgeStart, const glm::vec2& edgeEnd, const glm::vec2& point)
{
	return (edgeEnd.x - edgeStart.x) * (point.y - edgeStart.y) - (edgeEnd.y - edgeStart.y) * (point.x - edgeStart.x);
}

void Pipeline::DrawTriangle(const Varyings& vertexA, const Varyings& vertexB, const Varyings& vertexC)
{
	if (vertexA.position.w <= 0 || vertexB.position.w <= 0 || vertexC.position.w <= 0)
		return;

	const int   width   = m_target->width;
	const int   height  = m_target->height;
	const float fWidth  = (float)width;
	const float fHeight = (float)height;
	
	// Pour la perspective (clip space -> NDC)
	glm::vec3 ndcA = glm::vec3(vertexA.position) / vertexA.position.w;
	glm::vec3 ndcB = glm::vec3(vertexB.position) / vertexB.position.w;
	glm::vec3 ndcC = glm::vec3(vertexC.position) / vertexC.position.w;

	// Transformation viewport (NDC -> pixels)
	glm::vec3 screenA = glm::vec3((ndcA.x * 0.5f + 0.5f) * fWidth, (1.0f - (ndcA.y * 0.5f + 0.5f)) * fHeight, ndcA.z);
	glm::vec3 screenB = glm::vec3((ndcB.x * 0.5f + 0.5f) * fWidth, (1.0f - (ndcB.y * 0.5f + 0.5f)) * fHeight, ndcB.z);
	glm::vec3 screenC = glm::vec3((ndcC.x * 0.5f + 0.5f) * fWidth, (1.0f - (ndcC.y * 0.5f + 0.5f)) * fHeight, ndcC.z);

	// Back-face culling
	float area = EdgeFunction(screenA, screenB, screenC);
	if (area == 0.0f) return;
	if (m_cull == CullMode::Back  && area > 0.0f) return;
	if (m_cull == CullMode::Front && area < 0.0f) return;

	// Boite englobante du triangle
	int minX = (int)glm::clamp(glm::min(glm::min(screenA.x, screenB.x), screenC.x), 0.0f, fWidth - 1);
	int maxX = (int)glm::clamp(glm::max(glm::max(screenA.x, screenB.x), screenC.x), 0.0f, fWidth - 1);
	int minY = (int)glm::clamp(glm::min(glm::min(screenA.y, screenB.y), screenC.y), 0.0f, fHeight - 1);
	int maxY = (int)glm::clamp(glm::max(glm::max(screenA.y, screenB.y), screenC.y), 0.0f, fHeight - 1);

	const float invArea = 1.0f / area;

	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			glm::vec2 pixelCenter(x + 0.5f, y + 0.5f);

			// Rasterisation
			float weightA = EdgeFunction(screenB, screenC, pixelCenter) * invArea;
			float weightB = EdgeFunction(screenC, screenA, pixelCenter) * invArea;
			float weightC = EdgeFunction(screenA, screenB, pixelCenter) * invArea;

			// Poids negatif = pixel hors du triangle.
			// La tolerance evite les trous sur l'arete commune de deux triangles (erreurs d'arrondi) ;
			// un pixel dessine deux fois est ecarte par le z-buffer.
			const float edgeTolerance = -1e-5f;
			if (weightA < edgeTolerance || weightB < edgeTolerance || weightC < edgeTolerance)
				continue;

			// Depth Test (z-buffer)
			float depth = weightA * screenA.z + weightB * screenB.z + weightC * screenC.z;
			const size_t index = (size_t)y * width + x;
			if (depth >= m_target->depth[index])
				continue;

			m_target->depth[index] = depth;

			if (!m_pixelShader)
				continue;
			
			// Interpolation et correction perspective
			float perspectiveA = weightA / vertexA.position.w;
			float perspectiveB = weightB / vertexB.position.w;
			float perspectiveC = weightC / vertexC.position.w;

			float invPerspectiveSum = 1.0f / (perspectiveA + perspectiveB + perspectiveC);

			perspectiveA *= invPerspectiveSum;
			perspectiveB *= invPerspectiveSum;
			perspectiveC *= invPerspectiveSum;

			Varyings fragment;
			fragment.position = glm::vec4(pixelCenter.x, pixelCenter.y, depth, 1.0f);
			fragment.normal = perspectiveA * vertexA.normal + perspectiveB * vertexB.normal + perspectiveC * vertexC.normal;
			fragment.color = perspectiveA * vertexA.color + perspectiveB * vertexB.color + perspectiveC * vertexC.color;
			fragment.uv = perspectiveA * vertexA.uv + perspectiveB * vertexB.uv + perspectiveC * vertexC.uv;
			fragment.lightPos = perspectiveA * vertexA.lightPos + perspectiveB * vertexB.lightPos + perspectiveC * vertexC.lightPos;

			glm::vec3 pixelColor = m_pixelShader(fragment, m_uniforms);
			pixelColor.x = static_cast<float>(glm::clamp((int)pixelColor.x, 0, 255));
			pixelColor.y = static_cast<float>(glm::clamp((int)pixelColor.y, 0, 255));
			pixelColor.z = static_cast<float>(glm::clamp((int)pixelColor.z, 0, 255));

			if (m_target->color)
				m_target->color->put_pixel(x, y, (int)pixelColor.r << 16 | (int)pixelColor.g << 8 | (int)pixelColor.b);
		}
	}
}
