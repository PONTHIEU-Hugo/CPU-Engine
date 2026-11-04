#ifndef GEOMETRY_FACTORY_HPP_INCLUDED
#define GEOMETRY_FACTORY_HPP_INCLUDED

#include "core.h" // uint32, vec3, RANDOM ...
#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace GeometryFactory
{
	namespace detail
	{
		inline glm::vec4 White() { return glm::vec4(255.0f); }

		inline Vertex MakeVertex(const glm::vec3& _pos, const glm::vec2& _uv)
		{
			Vertex v;
			v.position  = _pos;
			v.normal = { 0.0f, 0.0f, 1.0f };
			v.color		= White();
			v.uv		= _uv;
			return v;
		}

		inline glm::vec3 SafeNormalize(const glm::vec3& _v, const glm::vec3& _fallback)
		{
			float len2 = dot(_v, _v);
			return len2 > 1e-12f ? _v * (1.0f / std::sqrt(len2)) : _fallback;
		}

		inline glm::vec3 OrthogonalizeTangent(const glm::vec3& _t, const glm::vec3& _n)
		{
			glm::vec3 t = _t - dot(_t, _n) * _n;
			if (glm::dot(t, t) > 1e-12f)
				return normalize(t);

			glm::vec3 helper = std::abs(_n.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
			return glm::normalize(cross(helper, _n));
		}

		inline void ComputeTangents(std::vector<Vertex>& vertices)
		{
			for (size_t i = 0; i + 2 < vertices.size(); i += 3)
			{
				Vertex& a = vertices[i];
				Vertex& b = vertices[i + 1];
				Vertex& c = vertices[i + 2];

				glm::vec3 e1 = b.position - a.position;
				glm::vec3 e2 = c.position - a.position;
				glm::vec2 d1 = b.uv - a.uv;
				glm::vec2 d2 = c.uv - a.uv;

				glm::vec3 t(1.0f, 0.0f, 0.0f);
				float w = 1.0f;

				float det = d1.x * d2.y - d2.x * d1.y;
				if (std::abs(det) > 1e-12f)
				{
					t = (e1 * d2.y - e2 * d1.y) / det;              
					glm::vec3 dPdV = (e2 * d1.x - e1 * d2.x) / det;

					w = glm::dot(glm::cross(a.normal, t), -dPdV) < 0.0f ? -1.0f : 1.0f;
				}

				t = SafeNormalize(t, glm::vec3(1, 0, 0));
				a.tangent = b.tangent = c.tangent = glm::vec4(t, w);
			}
		}
		
		inline void PushTriangle(std::vector<Vertex>& _out, const Vertex& a, const Vertex& b, const Vertex& c)
		{
			_out.push_back(a);
			_out.push_back(b);
			_out.push_back(c);
		}

		inline void AddTriangle(std::vector<Vertex>& _out,
						const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
		{
			PushTriangle(_out,
				MakeVertex(p0, { -1.0f, 1.0f }),
				MakeVertex(p1, { 0.0f, 1.0f }),
				MakeVertex(p2, { -0.5f, 0.0f }));
		}

		inline void AddQuad(std::vector<Vertex>& _out,
					const glm::vec3& p0, const glm::vec3& p1,
					const glm::vec3& p2, const glm::vec3& p3)
		{
			Vertex v0 = MakeVertex(p0, { -1.0f, 1.0f });
			Vertex v1 = MakeVertex(p1, { 0.0f, 1.0f });
			Vertex v2 = MakeVertex(p2, { 0.0f, 0.0f });
			Vertex v3 = MakeVertex(p3, { -1.0f, 0.0f });

			PushTriangle(_out, v0, v1, v2);
			PushTriangle(_out, v0, v2, v3);
		}
	}

	////////////////////////////////////////////
	//// NORMALES / TANGENTES
	////////////////////////////////////////////

	inline void CalculateNormals(std::vector<Vertex>& _vertices)
	{
		for (size_t i = 0; i + 2 < _vertices.size(); i += 3)
		{
			Vertex& v0 = _vertices[i];
			Vertex& v1 = _vertices[i + 1];
			Vertex& v2 = _vertices[i + 2];

			glm::vec3 e1 = v1.position - v0.position;
			glm::vec3 e2 = v2.position - v0.position;

			glm::vec3 N = detail::SafeNormalize(cross(e1, e2), glm::vec3(0, 1, 0));

			v0.normal = v1.normal = v2.normal = N;
		}
	}
	////////////////////////////////////////////
	//// SQUARE
	////////////////////////////////////////////
	
	inline Mesh BuildPlane()
	{
		std::vector<Vertex> vertices;

		Vertex a = detail::MakeVertex({-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f});
		Vertex b = detail::MakeVertex({ 0.5f, 0.0f, -0.5f}, {1.0f, 1.0f});
		Vertex c = detail::MakeVertex({ 0.5f, 0.0f,  0.5f}, {1.0f, 0.0f});
		Vertex d = detail::MakeVertex({-0.5f, 0.0f,  0.5f}, {0.0f, 0.0f});

		detail::PushTriangle(vertices, a, d, c);
		detail::PushTriangle(vertices, a, c, b);

		CalculateNormals(vertices);
		detail::ComputeTangents(vertices);
		
		
		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}
	
	////////////////////////////////////////////
	//// TRIANGLE
	////////////////////////////////////////////

	inline Mesh BuildTriangle()
	{
		std::vector<Vertex> vertices;

		Vertex bottomLeft  = detail::MakeVertex({ -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f });
		Vertex bottomRight = detail::MakeVertex({  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f });
		Vertex top         = detail::MakeVertex({  0.0f,  0.5f, 0.0f }, { 0.5f, 0.0f });

		detail::PushTriangle(vertices, bottomLeft, bottomRight, top);

		detail::PushTriangle(vertices, bottomLeft, top, bottomRight);

		CalculateNormals(vertices);
		detail::ComputeTangents(vertices);
		
		
		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// PYRAMID
	////////////////////////////////////////////

	inline Mesh BuildPyramid()
	{
		std::vector<Vertex> vertices;

		const glm::vec3 A(-0.5f, -0.5f, -0.5f);
		const glm::vec3 B( 0.5f, -0.5f, -0.5f);
		const glm::vec3 C( 0.5f, -0.5f,  0.5f);
		const glm::vec3 D(-0.5f, -0.5f,  0.5f);
		const glm::vec3 T( 0.0f,  0.5f,  0.0f);

		detail::AddTriangle(vertices, B, A, T); // avant  (-Z)
		detail::AddTriangle(vertices, C, B, T); // droite (+X)
		detail::AddTriangle(vertices, D, C, T); // arriere(+Z)
		detail::AddTriangle(vertices, A, D, T); // gauche (-X)
		detail::AddQuad(vertices, A, B, C, D);  // base   (-Y)

		CalculateNormals(vertices);
		detail::ComputeTangents(vertices);
		
		
		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// CUBE
	////////////////////////////////////////////

	inline Mesh BuildCube()
	{
		std::vector<Vertex> vertices;

		detail::AddQuad(vertices, { 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f}); // +X
		detail::AddQuad(vertices, {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f}); // -X
		detail::AddQuad(vertices, {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}); // +Y
		detail::AddQuad(vertices, {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f}); // -Y
		detail::AddQuad(vertices, {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}); // +Z
		detail::AddQuad(vertices, { 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}); // -Z

		CalculateNormals(vertices);
		detail::ComputeTangents(vertices);
		
		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// ICOSPHERE
	////////////////////////////////////////////

	inline Mesh BuildIcosphere(int _subdivisions, float _radius = 0.5f)
	{
		_subdivisions = glm::clamp(_subdivisions, 0, 6);

		using Tri = std::array<glm::vec3, 3>;

		const float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

		std::vector<glm::vec3> p = {
			{-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
			{ 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
			{ t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1},
		};
		for (auto& v : p)
			v = normalize(v);

		// 20 faces, CCW vues de l'exterieur
		static const int faces[20][3] = {
			{0, 11, 5}, {0, 5, 1},  {0, 1, 7},  {0, 7, 10}, {0, 10, 11},
			{1, 5, 9},  {5, 11, 4}, {11, 10, 2},{10, 7, 6}, {7, 1, 8},
			{3, 9, 4},  {3, 4, 2},  {3, 2, 6},  {3, 6, 8},  {3, 8, 9},
			{4, 9, 5},  {2, 4, 11}, {6, 2, 10}, {8, 6, 7},  {9, 8, 1},
		};

		std::vector<Tri> tris;
		for (auto& f : faces)
			tris.push_back({ p[f[0]], p[f[1]], p[f[2]] });

		for (int s = 0; s < _subdivisions; ++s)
		{
			std::vector<Tri> next;
			next.reserve(tris.size() * 4);

			for (const Tri& tri : tris)
			{
				glm::vec3 m0 = normalize((tri[0] + tri[1]) * 0.5f);
				glm::vec3 m1 = normalize((tri[1] + tri[2]) * 0.5f);
				glm::vec3 m2 = normalize((tri[2] + tri[0]) * 0.5f);

				next.push_back({ tri[0], m0, m2 });
				next.push_back({ tri[1], m1, m0 });
				next.push_back({ tri[2], m2, m1 });
				next.push_back({ m0, m1, m2 });
			}
			tris.swap(next);
		}

		std::vector<Vertex> vertices;
		vertices.reserve(tris.size() * 3);

		for (const Tri& tri : tris)
		{
			float u[3], v[3];
			bool  isPole[3];

			for (int k = 0; k < 3; ++k)
			{
				const glm::vec3& n = tri[k];

				float angle = std::atan2(n.z, n.x);           
				if (angle < 0.0f) angle += 2.0f * glm::pi<float>();     
				u[k] = -angle / (2.0f * glm::pi<float>());               

				v[k] = 0.5f - std::asin(glm::clamp(n.y, -1.0f, 1.0f)) / glm::pi<float>();
				isPole[k] = std::abs(n.y) > 0.9999f;
			}

			float minU = 2.0f, maxU = -1.0f;
			for (int k = 0; k < 3; ++k)
			{
				if (isPole[k]) continue;
				minU = std::min(minU, u[k]);
				maxU = std::max(maxU, u[k]);
			}
			if (maxU - minU > 0.5f)
				for (int k = 0; k < 3; ++k)
					if (maxU - minU > 0.5f)
						for (int k = 0; k < 3; ++k)
							if (!isPole[k] && u[k] > -0.5f)
								u[k] -= 1.0f;

			for (int k = 0; k < 3; ++k)
				if (isPole[k])
					u[k] = (u[(k + 1) % 3] + u[(k + 2) % 3]) * 0.5f;

			for (int k = 0; k < 3; ++k)
			{
				const glm::vec3& n = tri[k];
				Vertex vertex = detail::MakeVertex(
						n * _radius,
						{ u[k], v[k] }
					);

				vertex.normal = n;
				glm::vec3 t(-n.z, 0.0f, n.x);
				vertex.tangent = glm::vec4(detail::SafeNormalize(t, glm::vec3(1, 0, 0)), -1.0f);
				vertices.push_back(vertex);
			}
		}

		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// UV SPHERE
	////////////////////////////////////////////

	inline Mesh BuildUVSphere(uint32_t _sliceCount, uint32_t _stackCount, float _radius = 0.5f)
	{
		_sliceCount = std::max<uint32_t>(_sliceCount, 3);
		_stackCount = std::max<uint32_t>(_stackCount, 2);

		const float phiStep   = glm::pi<float>() / (float)_stackCount;
		const float thetaStep = 2.0f * glm::pi<float>() / (float)_sliceCount;

		auto sphereVertex = [&](uint32_t i, uint32_t j) -> Vertex
		{
			float phi   = (float)i * phiStep;
			float theta = (float)j * thetaStep;

			glm::vec3 n(std::sin(phi) * std::cos(theta),
			            std::cos(phi),
			            std::sin(phi) * std::sin(theta));
			glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));
			glm::vec2 uv(-(float)j / (float)_sliceCount, (float)i / (float)_stackCount);

			Vertex vertex = detail::MakeVertex(n * _radius, uv);
			vertex.normal = n;
			vertex.tangent = glm::vec4(tangent, -1.0f);
			return vertex;
		};

		std::vector<Vertex> vertices;

		for (uint32_t i = 0; i < _stackCount; ++i)
		{
			for (uint32_t j = 0; j < _sliceCount; ++j)
			{
				Vertex a = sphereVertex(i,     j);
				Vertex b = sphereVertex(i,     j + 1);
				Vertex c = sphereVertex(i + 1, j);
				Vertex d = sphereVertex(i + 1, j + 1);

				float uMid = -((float)j + 0.5f) / (float)_sliceCount;

				if (i == 0)
				{
					// calotte nord : a et b sont le pole
					b.uv.x = uMid;
					detail::PushTriangle(vertices, c, b, d);
				}
				else if (i == _stackCount - 1)
				{
					// calotte sud : c et d sont le pole
					c.uv.x = uMid;
					detail::PushTriangle(vertices, a, b, c);
				}
				else
				{
					detail::PushTriangle(vertices, a, b, c);
					detail::PushTriangle(vertices, c, b, d);
				}
			}
		}
		
		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// CIRCLE (disque)
	////////////////////////////////////////////

	inline void AddCircle(std::vector<Vertex>& _vertices, int _stackCount, float _posY, bool _isTop)
	{

		const float angle = 2.0f * glm::pi<float>() / (float)_stackCount;

		Vertex center = detail::MakeVertex({ 0.0f, _posY, 0.0f }, { 0.5f, 0.5f });

		center.normal = _isTop
				? glm::vec3(0.0f, 1.0f, 0.0f)
				: glm::vec3(0.0f, -1.0f, 0.0f);
		auto ringVertex = [&](int i) -> Vertex
		{
			float c = std::cos(angle * (float)i);
			float s = std::sin(angle * (float)i);
			
			Vertex vertex = detail::MakeVertex(
					{ c * 0.5f, _posY, s * 0.5f },
					{ 0.5f + c * 0.5f, 0.5f + s * 0.5f }
				);

			vertex.normal = _isTop
				? glm::vec3(0.0f, 1.0f, 0.0f)
				: glm::vec3(0.0f, -1.0f, 0.0f);

			return vertex;
		};

		for (int i = 0; i < _stackCount; ++i)
		{
			Vertex a = ringVertex(i);
			Vertex b = ringVertex(i + 1);

			if (_isTop) detail::PushTriangle(_vertices, center, b, a);
			else        detail::PushTriangle(_vertices, center, a, b);
		}
	}

	inline Mesh BuildCircle(int _stackCount, float _posY, bool _isTop)
	{
		std::vector<Vertex> vertices;
		AddCircle(vertices, std::max(_stackCount, 3), _posY, _isTop);
		CalculateNormals(vertices);
		
		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// CYLINDER
	////////////////////////////////////////////

	inline Mesh BuildCylinder(int _stackCount)
	{
		_stackCount = std::max(_stackCount, 3);

		std::vector<Vertex> vertices;
		AddCircle(vertices, _stackCount, -0.5f, false);
		AddCircle(vertices, _stackCount,  0.5f, true);

		const float angle = 2.0f * glm::pi<float>() / (float)_stackCount;

		auto sideVertex = [&](int i, float y) -> Vertex
		{
			float u = (float)i / (float)_stackCount;
			float angle = 2.0f * glm::pi<float>() * u;

			float c = std::cos(angle);
			float s = std::sin(angle);

			Vertex vertex = detail::MakeVertex(
				{ c * 0.5f, y, s * 0.5f },
				{ u, y < 0.0f ? 1.0f : 0.0f }
			);

			vertex.normal = glm::vec3(c, 0.0f, s);
			vertex.tangent = glm::vec4(-s, 0.0f, c, -1.0f);

			return vertex;
		};

		for (int i = 0; i < _stackCount; ++i)
		{
			Vertex bottom0 = sideVertex(i,     -0.5f);
			Vertex top0    = sideVertex(i,      0.5f);
			Vertex bottom1 = sideVertex(i + 1, -0.5f);
			Vertex top1    = sideVertex(i + 1,  0.5f);

			detail::PushTriangle(vertices, bottom0, top0,    bottom1);
			detail::PushTriangle(vertices, top0,    top1,    bottom1);
		}

		Mesh geometry;
		geometry.vertices = std::move(vertices); 
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// DONUT (tore)
	////////////////////////////////////////////

	inline Mesh BuildDonut(float _majorRadius = 0.35f, float _minorRadius = 0.15f,
						   uint32_t _majorSlices = 32, uint32_t _minorSlices = 16)
	{
		_majorSlices = std::max<uint32_t>(_majorSlices, 3);
		_minorSlices = std::max<uint32_t>(_minorSlices, 3);

		const float twoPi = 2.0f * glm::pi<float>();

		auto torusVertex = [&](uint32_t i, uint32_t j) -> Vertex
		{
			float u = (float)i / (float)_majorSlices;
			float v = (float)j / (float)_minorSlices;

			float theta = u * twoPi; 
			float phi   = v * twoPi;

			float cosT = std::cos(theta), sinT = std::sin(theta);
			float cosP = std::cos(phi),   sinP = std::sin(phi);

			glm::vec3 n(cosP * cosT, sinP, cosP * sinT);
			glm::vec3 pos((_majorRadius + _minorRadius * cosP) * cosT,
						   _minorRadius * sinP,
						  (_majorRadius + _minorRadius * cosP) * sinT);

			Vertex vertex = detail::MakeVertex(pos, { u, v });
			vertex.normal = n;
			vertex.tangent = glm::vec4(-sinT, 0.0f, cosT, 1.0f);
			return vertex;
		};

		std::vector<Vertex> vertices;
		vertices.reserve((size_t)_majorSlices * _minorSlices * 6);

		for (uint32_t i = 0; i < _majorSlices; ++i)
		{
			for (uint32_t j = 0; j < _minorSlices; ++j)
			{
				Vertex a = torusVertex(i,     j);
				Vertex b = torusVertex(i,     j + 1);
				Vertex c = torusVertex(i + 1, j);
				Vertex d = torusVertex(i + 1, j + 1);

				detail::PushTriangle(vertices, a, b, c);
				detail::PushTriangle(vertices, c, b, d);
			}
		}

		Mesh geometry;
		geometry.vertices = std::move(vertices);
		geometry.ComputeBounds();
		return geometry;
	}

	////////////////////////////////////////////
	//// OBJ LOADER
	////////////////////////////////////////////

	inline Mesh LoadGeometry(std::string _path)
    {
		std::vector<Vertex> vertices;
    	
    	std::vector<glm::vec3> rawPositions;
    	std::vector<glm::vec2> rawUVs;
		std::vector<glm::vec3> rawNormals;

    	FILE* file = nullptr;
    	errno_t err = fopen_s(&file, _path.c_str(), "r");
    	if (err != 0 || file == nullptr)
    	{
    		return {};
    	}

    	while (1)
    	{
    		char lineHeader[128];
    		int res = fscanf_s(file, "%s", lineHeader, (unsigned)_countof(lineHeader));
    		if (res == EOF)
    			break;

    		if (strcmp(lineHeader, "v") == 0)
    		{
				glm::vec3 pos;
    			fscanf_s(file, "%f %f %f\n", &pos.x, &pos.y, &pos.z);
    			pos.z = -pos.z;
    			rawPositions.push_back(pos);
    		}
    		else if (strcmp(lineHeader, "vn") == 0)
    		{
				glm::vec3 normal;
    			fscanf_s(file, "%f %f %f",
					&normal.x, &normal.y, &normal.z);

    			normal.z = -normal.z;

    			rawNormals.push_back(normal);
    		}
    		else if (strcmp(lineHeader, "vt") == 0)
    		{
				glm::vec2 uv;
    			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
    			uv.y = 1 - uv.y;
    			rawUVs.push_back(uv);
    		}
    		else if (strcmp(lineHeader, "f") == 0)
    		{
    			int posIdx[3], uvIdx[3], normalIdx[3];
    			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&posIdx[0], &uvIdx[0], &normalIdx[0],
					&posIdx[1], &uvIdx[1], &normalIdx[1],
					&posIdx[2], &uvIdx[2], &normalIdx[2]);

    			if (matches != 9)
    			{
    				printf("File can't be read by our simple parser\n");
    				fclose(file);
    				return {};
    			}

    			int order[3] = {0, 2, 1};
    			for (int k = 0; k < 3; ++k)
    			{
    				int idx = order[k];
    				Vertex v;
    				v.position = rawPositions[posIdx[idx] - 1];
    				v.uv       = rawUVs.empty() ? glm::vec2() : rawUVs[uvIdx[idx] - 1];
    				v.normal = rawNormals[normalIdx[idx] - 1];
    				v.color = detail::White();
    				vertices.push_back(v);
    			}
    		}
    	}

    	fclose(file);

		detail::ComputeTangents(vertices);
		
		Mesh geometry;
		geometry.vertices = std::move(vertices);
		geometry.ComputeBounds();
		return geometry;
	}

	inline Mesh BuildSkybox()
	{
		static const glm::vec3 p[8] = 
		{
			{-1,-1,-1},{ 1,-1,-1},{ 1, 1,-1},{-1, 1,-1},
			{-1,-1, 1},{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1} 
		};
		static const int idx[36] = 
		{
			0,1,2, 0,2,3,  5,4,7, 5,7,6,  4,0,3, 4,3,7,
			1,5,6, 1,6,2,  3,2,6, 3,6,7,  4,5,1, 4,1,0 
		};

		Mesh m;
		for (int i : idx)
		{
			Vertex v{};
			v.position = p[i];
			v.normal = -glm::normalize(p[i]);
			v.tangent = glm::vec4(1, 0, 0, 1);
			m.vertices.push_back(v);
		}
		return m;
	}
}

#endif 