#include "Pipeline.h"
#include "Clipping.h"
#include "core.h"
#include "Mesh.h"
#include <algorithm>
#include <omp.h>

#include "Texture.h"

namespace
{
	constexpr float kPoleEps = 1e-4f;
	constexpr float kEdgeTolerance = -1e-5f;

	float EdgeFunction(const glm::vec2& edgeStart, const glm::vec2& edgeEnd, const glm::vec2& point)
	{
		return (edgeEnd.x - edgeStart.x) * (point.y - edgeStart.y) - (edgeEnd.y - edgeStart.y) * (point.x - edgeStart.x);
	}

	// Limites d'une tuile en pixels (bornes inclusives)
	void TileBounds(int tile, int width, int height, int& x0, int& y0, int& x1, int& y1)
	{
		const int tx = tile % Pipeline::TILES_X;
		const int ty = tile / Pipeline::TILES_X;
		x0 = tx * width / Pipeline::TILES_X;
		x1 = (tx + 1) * width / Pipeline::TILES_X - 1;
		y0 = ty * height / Pipeline::TILES_Y;
		y1 = (ty + 1) * height / Pipeline::TILES_Y - 1;
	}
	
	bool PosLess(const glm::vec3& a, const glm::vec3& b)
	{
		if (a.x != b.x) return a.x < b.x;
		if (a.y != b.y) return a.y < b.y;
		return a.z < b.z;
	}

	bool IsPoleUV(const glm::vec2& uv)
	{
		return uv.y < kPoleEps || uv.y > 1.0f - kPoleEps;
	}
	
	Vertex LerpV(const Vertex& a, const Vertex& b, float t)
	{
		Vertex r;
		r.position = glm::mix(a.position, b.position, t);
		r.normal   = glm::normalize(glm::mix(a.normal, b.normal, t));
		r.color    = glm::mix(a.color, b.color, t);
		r.uv       = glm::mix(a.uv, b.uv, t);
		const bool poleA = IsPoleUV(a.uv), poleB = IsPoleUV(b.uv);
		if (poleA && !poleB)      r.uv.x = b.uv.x;
		else if (poleB && !poleA) r.uv.x = a.uv.x;

		r.tangent  = glm::vec4(glm::normalize(glm::mix(glm::vec3(a.tangent), glm::vec3(b.tangent), t)), a.tangent.w);
		return r;
	}

	Vertex EdgePoint(Vertex p0, Vertex p1, float t)
	{
		if (PosLess(p1.position, p0.position)) { std::swap(p0, p1); t = 1.0f - t; }
		return LerpV(p0, p1, t);
	}
}

Pipeline::Pipeline() : m_screen(WIDTH, HEIGHT)
{
	m_target = &m_screen;
	m_threadBins.resize(TILE_COUNT);
}

void Pipeline::ClearDepth()
{
	if (m_target) m_target->ClearDepth();
}

void Pipeline::Draw(glm::mat4 const& mat)
{
    if (!m_mesh || !m_target || !m_vertexShader) return;
    if (m_target->color && !m_pixelShader) return;

    const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat)));
    const std::vector<Vertex>& src = m_mesh->vertices;
    const int srcCount = (int)src.size();

    m_worldVertices.resize(srcCount);
    
#pragma omp parallel for schedule(static)
    for (int i = 0; i < srcCount; ++i)
    {
        Vertex v = src[i];
        v.position = glm::vec3(mat * glm::vec4(src[i].position, 1.0f));
        v.normal   = glm::normalize(normalMatrix * src[i].normal);
        v.tangent  = glm::vec4(glm::normalize(glm::mat3(mat) * glm::vec3(src[i].tangent)), src[i].tangent.w);
        m_worldVertices[i] = v;
    }

    const std::vector<Vertex>* source = &m_worldVertices;
    if (m_uniforms.displacementMap && m_uniforms.displacementScale != 0.0f && m_uniforms.displacementMap->IsValid())
    {
        const int N = ComputeTessLevel(m_worldVertices);
        const int tris = srcCount / 3;
        const int nt = omp_get_max_threads();
        std::vector<std::vector<Vertex>> perThread(nt);

#pragma omp parallel
        {
            auto& local = perThread[omp_get_thread_num()];
            local.reserve((size_t)tris / nt * N * N * 3 + 3);
#pragma omp for schedule(dynamic, 5)
            for (int t = 0; t < tris; ++t)
                TessellateTriangle(m_worldVertices[3*t], m_worldVertices[3*t+1], m_worldVertices[3*t+2], N, local);
        }

        m_tessVertices.clear();
        for (auto& l : perThread)
            m_tessVertices.insert(m_tessVertices.end(), l.begin(), l.end());
        source = &m_tessVertices;
    }

    const int vertexCount = (int)source->size();
    const int triangleCount = vertexCount / 3;
    m_varyings.resize(vertexCount);

#pragma omp parallel for schedule(static)
    for (int i = 0; i < vertexCount; ++i)
        m_varyings[i] = m_vertexShader((*source)[i], m_uniforms);

    // Clipping + setup + binning 
    const int threadCount = omp_get_max_threads();
    if ((int)m_threadBins.size() < threadCount)
       m_threadBins.resize(threadCount);

    for (ThreadBins& tb : m_threadBins)
    {
       tb.triangles.clear();
       for (std::vector<int>& bin : tb.bins)
          bin.clear();
    }

#pragma omp parallel
    {
       ThreadBins& localThread = m_threadBins[omp_get_thread_num()];

#pragma omp for schedule(static)
       for (int t = 0; t < triangleCount; ++t)
       {
          Varyings poly[12];
          const int count = ClipTriangle(m_varyings[3 * t], m_varyings[3 * t + 1], m_varyings[3 * t + 2], poly);

          for (int k = 1; k + 1 < count; ++k)
          {
             ScreenTriangle triangle;
             if (SetupTriangle(poly[0], poly[k], poly[k + 1], triangle) == false)
                continue;

             const int index = (int)localThread.triangles.size();
             localThread.triangles.push_back(triangle);
             BinTriangle(localThread.triangles.back(), index, localThread);
          }
       }
    }

    const int width = m_target->width;
    const int height = m_target->height;

#pragma omp parallel for schedule(dynamic)
    for (int tile = 0; tile < TILE_COUNT; ++tile)
    {
       int x0, y0, x1, y1;
       TileBounds(tile, width, height, x0, y0, x1, y1);

       for (int th = 0; th < threadCount; ++th)
       {
          const ThreadBins& tb = m_threadBins[th];
          for (int index : tb.bins[tile])
             RasterizeInTile(tb.triangles[index], x0, y0, x1, y1);
       }
    }
}

bool Pipeline::SetupTriangle(const Varyings& a, const Varyings& b, const Varyings& c, ScreenTriangle& out) const
{
	if (a.position.w <= 0 || b.position.w <= 0 || c.position.w <= 0)
		return false;

	const float fWidth = (float)m_target->width;
	const float fHeight = (float)m_target->height;

	const Varyings* src[3] = { &a, &b, &c };
	for (int k = 0; k < 3; ++k)
	{
		const glm::vec4& p = src[k]->position;

		// Perspective (clip space -> NDC)
		const glm::vec3 ndc = glm::vec3(p) / p.w;

		// Viewport (NDC -> pixels)
		out.varyings[k] = *src[k];
		out.screenPos[k] = glm::vec3((ndc.x * 0.5f + 0.5f) * fWidth, (1.0f - (ndc.y * 0.5f + 0.5f)) * fHeight, ndc.z);
	}

	const glm::vec2 screenA(out.screenPos[0]), screenB(out.screenPos[1]), screenC(out.screenPos[2]);

	// Back-face culling
	const float area = EdgeFunction(screenA, screenB, screenC);
	if (area == 0.0f) return false;
	if (m_cull == CullMode::Back && area > 0.0f) return false;
	if (m_cull == CullMode::Front && area < 0.0f) return false;

	// Boite englobante ; rejet si le triangle est entierement hors ecran
	const float minXf = std::min({ screenA.x, screenB.x, screenC.x });
	const float maxXf = std::max({ screenA.x, screenB.x, screenC.x });
	const float minYf = std::min({ screenA.y, screenB.y, screenC.y });
	const float maxYf = std::max({ screenA.y, screenB.y, screenC.y });
	if (maxXf < 0.0f || maxYf < 0.0f || minXf >= fWidth || minYf >= fHeight)
		return false;

	out.minX = (int)glm::clamp(minXf, 0.0f, fWidth - 1);
	out.maxX = (int)glm::clamp(maxXf, 0.0f, fWidth - 1);
	out.minY = (int)glm::clamp(minYf, 0.0f, fHeight - 1);
	out.maxY = (int)glm::clamp(maxYf, 0.0f, fHeight - 1);

	out.invArea = 1.0f / area;
	return true;
}

void Pipeline::BinTriangle(const ScreenTriangle& tri, int index, ThreadBins& local) const
{
	const int width = m_target->width;
	const int height = m_target->height;

	const glm::vec2 screenA(tri.screenPos[0]), screenB(tri.screenPos[1]), screenC(tri.screenPos[2]);

	// poids A = arete (B,C), poids B = arete (C,A), poids C = arete (A,B)
	const glm::vec2 edgeStart[3] = { screenB, screenC, screenA };
	const glm::vec2 edgeEnd[3] = { screenC, screenA, screenB };

	for (int tile = 0; tile < TILE_COUNT; ++tile)
	{
		int x0, y0, x1, y1;
		TileBounds(tile, width, height, x0, y0, x1, y1);

		// Boites englobantes
		if (tri.maxX < x0 || tri.minX > x1 || tri.maxY < y0 || tri.minY > y1)
			continue;

		// Les 3 aretes
		bool outside = false;
		for (int e = 0; e < 3 && !outside; ++e)
		{
			// Gradient du poids (fonction d'arete * invArea) : son signe gere les deux sens d'enroulement
			const float gradX = -(edgeEnd[e].y - edgeStart[e].y) * tri.invArea;
			const float gradY = (edgeEnd[e].x - edgeStart[e].x) * tri.invArea;

			const glm::vec2 bestPixel(gradX >= 0.0f ? x1 + 0.5f : x0 + 0.5f,
				gradY >= 0.0f ? y1 + 0.5f : y0 + 0.5f);

			if (EdgeFunction(edgeStart[e], edgeEnd[e], bestPixel) * tri.invArea < kEdgeTolerance)
				outside = true;
		}

		if (outside == false)
			local.bins[tile].push_back(index);
	}
}


int Pipeline::ComputeTessLevel(const std::vector<Vertex>& w) const
{
    const int tris = (int)w.size() / 3;
    if (tris == 0) return 1;
    const int step = std::max(1, tris / 64);
    const glm::vec2 half((float)m_target->width * 0.5f, (float)m_target->height * 0.5f);

    float sum = 0.0f; int n = 0;
    for (int t = 0; t < tris; t += step)
        for (int e = 0; e < 3; ++e)
        {
            glm::vec4 c0 = m_uniforms.vp * glm::vec4(w[3*t + e].position, 1.0f);
            glm::vec4 c1 = m_uniforms.vp * glm::vec4(w[3*t + (e+1)%3].position, 1.0f);
            if (c0.w <= 0.0f || c1.w <= 0.0f) continue;
            sum += glm::length((glm::vec2(c1) / c1.w - glm::vec2(c0) / c0.w) * half);
            ++n;
        }
    if (n == 0) return 1;
    return glm::clamp((int)std::ceil(sum / (float)n / m_uniforms.displacementPixelEdge), 1, 16);
}

void Pipeline::TessellateTriangle(const Vertex& a, const Vertex& b, const Vertex& c, int N, std::vector<Vertex>& out) const
{
    std::vector<Vertex> G((N + 1) * (N + 1));         
    auto at = [&](int i, int j) -> Vertex& { return G[i * (N + 1) + j]; };

	for (int i = 0; i <= N; ++i)
	{
		for (int j = 0; j <= N - i; ++j)
		{
			const int k = N - i - j;    
			Vertex p;

			if      (k == N) p = a;                                  
			else if (i == N) p = b;                                  
			else if (j == N) p = c;                                  
			else if (k == 0) p = EdgePoint(b, c, (float)j / N);      
			else if (i == 0) p = EdgePoint(a, c, (float)j / N);     
			else if (j == 0) p = EdgePoint(a, b, (float)i / N);    
			else
			{
				const float wb = (float)i / N, wc = (float)j / N, wa = (float)k / N;
				p.position = a.position * wa + b.position * wb + c.position * wc;
				p.normal   = glm::normalize(a.normal * wa + b.normal * wb + c.normal * wc);
				p.color    = a.color * wa + b.color * wb + c.color * wc;
				p.uv       = a.uv * wa + b.uv * wb + c.uv * wc;
				p.tangent  = glm::vec4(glm::normalize(glm::vec3(a.tangent * wa + b.tangent * wb + c.tangent * wc)), a.tangent.w);
			}
			at(i, j) = p;
		}
	}
	
    for (int i = 0; i < N; ++i)
    {
	    for (int j = 0; j < N - i; ++j)
	    {
	    	out.push_back(at(i, j));     
	    	out.push_back(at(i + 1, j));     
	    	out.push_back(at(i, j + 1));
	    	if (j < N - i - 1)
	    	{
	    		out.push_back(at(i + 1, j)); 
	    		out.push_back(at(i + 1, j + 1)); 
	    		out.push_back(at(i, j + 1));
	    	}
	    }
    }
}

void Pipeline::RasterizeInTile(const ScreenTriangle& tri, int tileX0, int tileY0, int tileX1, int tileY1)
{
	// Intersection boite du triangle / tuile : on ne sort jamais de la tuile
	const int minX = std::max(tri.minX, tileX0);
	const int maxX = std::min(tri.maxX, tileX1);
	const int minY = std::max(tri.minY, tileY0);
	const int maxY = std::min(tri.maxY, tileY1);

	const int width = m_target->width;

	const glm::vec3& screenA = tri.screenPos[0];
	const glm::vec3& screenB = tri.screenPos[1];
	const glm::vec3& screenC = tri.screenPos[2];
	const glm::vec2 sA(screenA), sB(screenB), sC(screenC);

	const Varyings& vertexA = tri.varyings[0];
	const Varyings& vertexB = tri.varyings[1];
	const Varyings& vertexC = tri.varyings[2];

	struct Attr { glm::vec2 uv; glm::vec3 pos; };
	
	auto attrAt = [&](const glm::vec2& p) -> Attr
	{
		float a = EdgeFunction(sB, sC, p) * tri.invArea / vertexA.position.w;
		float b = EdgeFunction(sC, sA, p) * tri.invArea / vertexB.position.w;
		float c = EdgeFunction(sA, sB, p) * tri.invArea / vertexC.position.w;
		const float inv = 1.0f / (a + b + c);
		return {
			(a * vertexA.uv       + b * vertexB.uv       + c * vertexC.uv)       * inv,
			(a * vertexA.worldPos + b * vertexB.worldPos + c * vertexC.worldPos) * inv
		};
	};
	
	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			const glm::vec2 pixelCenter(x + 0.5f, y + 0.5f);

			// Rasterisation
			const float weightA = EdgeFunction(sB, sC, pixelCenter) * tri.invArea;
			const float weightB = EdgeFunction(sC, sA, pixelCenter) * tri.invArea;
			const float weightC = EdgeFunction(sA, sB, pixelCenter) * tri.invArea;

			// Poids negatif = pixel hors du triangle
			if (weightA < kEdgeTolerance || weightB < kEdgeTolerance || weightC < kEdgeTolerance)
				continue;

			// Depth Test (z-buffer)
			const float depth = weightA * screenA.z + weightB * screenB.z + weightC * screenC.z;
			const size_t index = (size_t)y * width + x;
			if (depth >= m_target->depth[index])
				continue;

			if (!m_pixelShader)
			{
				if (m_depthWrite)
					m_target->depth[index] = depth;
				continue;
			}
			
			// Interpolation et correction perspective
			float perspectiveA = weightA / vertexA.position.w;
			float perspectiveB = weightB / vertexB.position.w;
			float perspectiveC = weightC / vertexC.position.w;

			const float invPerspectiveSum = 1.0f / (perspectiveA + perspectiveB + perspectiveC);

			perspectiveA *= invPerspectiveSum;
			perspectiveB *= invPerspectiveSum;
			perspectiveC *= invPerspectiveSum;

			Varyings fragment;
			fragment.position = glm::vec4(pixelCenter.x, pixelCenter.y, depth, 1.0f);
			if (m_lite == false)
			{
				fragment.normal = perspectiveA * vertexA.normal + perspectiveB * vertexB.normal + perspectiveC * vertexC.normal;
				fragment.tangent = perspectiveA * vertexA.tangent + perspectiveB * vertexB.tangent + perspectiveC * vertexC.tangent;
				fragment.worldPos = perspectiveA * vertexA.worldPos + perspectiveB * vertexB.worldPos + perspectiveC * vertexC.worldPos;
				fragment.lightPos = perspectiveA * vertexA.lightPos + perspectiveB * vertexB.lightPos + perspectiveC * vertexC.lightPos;
			}
			fragment.color = perspectiveA * vertexA.color + perspectiveB * vertexB.color + perspectiveC * vertexC.color;
			fragment.uv = perspectiveA * vertexA.uv + perspectiveB * vertexB.uv + perspectiveC * vertexC.uv;
			
			if (m_uniforms.texture || m_uniforms.normalMap)
			{
				const Attr r	= attrAt(pixelCenter + glm::vec2(1.0f, 0.0f));
				const Attr d	= attrAt(pixelCenter + glm::vec2(0.0f, 1.0f));
				fragment.duvdx	= r.uv  - fragment.uv;
				fragment.duvdy	= d.uv  - fragment.uv;
				fragment.dpdx	= r.pos - fragment.worldPos;
				fragment.dpdy	= d.pos - fragment.worldPos;
			}
			
			glm::vec4 shaded = m_pixelShader(fragment, m_uniforms);

			if (!m_target->tint.empty())
			{
				glm::vec3 c = glm::clamp(glm::vec3(shaded), 0.0f, 255.0f);
				m_target->tint[index] = (int)c.r << 16 | (int)c.g << 8 | (int)c.b;
				if (m_depthWrite)
					m_target->depth[index] = depth;
				continue;
			}
			
			if (!m_target->color)
				continue;

			glm::vec3 src = glm::clamp(glm::vec3(shaded), 0.0f, 255.0f);

			if (m_blend)
			{
				float a = glm::clamp(shaded.a / 255.0f, 0.0f, 1.0f);
				if (a <= 0.0f)
					continue;   // totalement transparent : ni lecture ni ecriture du pixel
				glm::vec3 dst = glm::vec3(ToColor(m_target->color->get_pixel(x, y)));
				glm::vec3 out = m_additive ? glm::min(dst + src * a, glm::vec3(255.0f))
				                           : src * a + dst * (1.0f - a);

				m_target->color->put_pixel(x, y, (int)out.r << 16 | (int)out.g << 8 | (int)out.b);
			}
			else
			{
				if (shaded.a < m_alphaCutoff)
					continue;
				if (src.g < 10.0f)
					int ki = 0;
				m_target->color->put_pixel(x, y, (int)src.r << 16 | (int)src.g << 8 | (int)src.b);
			}

			if (m_depthWrite)
				m_target->depth[index] = depth;
		}
	}
}
