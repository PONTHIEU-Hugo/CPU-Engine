#ifndef GEOMETRY_FACTORY_HPP_INCLUDED
#define GEOMETRY_FACTORY_HPP_INCLUDED

#include "core.h" // uint32, RANDOM::GetRandomInt32_t ...

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
		inline vec3 White() { return vec3(255.0f); }

		inline Vertex MakeVertex(const vec3& _pos, const vec2& _uv)
		{
			Vertex v;
			v.position = _pos;
			v.color    = White();
			v.uv       = _uv;
			return v;
		}

		inline vec3 SafeNormalize(const vec3& _v, const vec3& _fallback)
		{
			float len2 = dot(_v, _v);
			return len2 > 1e-12f ? _v * (1.0f / std::sqrt(len2)) : _fallback;
		}

		inline vec3 OrthogonalizeTangent(const vec3& _t, const vec3& _n)
		{
			vec3 t = _t - dot(_t, _n) * _n;
			if (dot(t, t) > 1e-12f)
				return normalize(t);

			vec3 helper = std::abs(_n.y) < 0.99f ? vec3(0, 1, 0) : vec3(1, 0, 0);
			return normalize(cross(helper, _n));
		}

		inline void PushTriangle(std::vector<Vertex>& _out, const Vertex& a, const Vertex& b, const Vertex& c)
		{
			_out.push_back(a);
			_out.push_back(b);
			_out.push_back(c);
		}

		// Triangle simple (p0, p1, p2 en CCW)
		inline void AddTriangle(std::vector<Vertex>& _out,
		                        const vec3& p0, const vec3& p1, const vec3& p2)
		{
			PushTriangle(_out,
				MakeVertex(p0, { 0.0f, 1.0f }),
				MakeVertex(p1, { 1.0f, 1.0f }),
				MakeVertex(p2, { 0.5f, 0.0f }));
		}

		// Quad = 2 triangles (p0..p3 en CCW)
		inline void AddQuad(std::vector<Vertex>& _out,
		                    const vec3& p0, const vec3& p1,
		                    const vec3& p2, const vec3& p3)
		{
			Vertex v0 = MakeVertex(p0, { 0.0f, 1.0f });
			Vertex v1 = MakeVertex(p1, { 1.0f, 1.0f });
			Vertex v2 = MakeVertex(p2, { 1.0f, 0.0f });
			Vertex v3 = MakeVertex(p3, { 0.0f, 0.0f });

			PushTriangle(_out, v0, v1, v2);
			PushTriangle(_out, v0, v2, v3);
		}
	}

	////////////////////////////////////////////
	//// NORMALES / TANGENTES
	////////////////////////////////////////////

	// Flat : une normale / tangente par triangle (les vertices sont deja dupliques).
	inline void CalculateNormalsAndTangents(std::vector<Vertex>& _vertices)
	{
		/*for (size_t i = 0; i + 2 < _vertices.size(); i += 3)
		{
			Vertex& v0 = _vertices[i];
			Vertex& v1 = _vertices[i + 1];
			Vertex& v2 = _vertices[i + 2];

			vec3 e1 = v1.position - v0.position;
			vec3 e2 = v2.position - v0.position;

			vec3 N = detail::SafeNormalize(cross(e1, e2), vec3(0, 1, 0));

			vec2 d1 = v1.uv - v0.uv;
			vec2 d2 = v2.uv - v0.uv;
			float denom = d1.x * d2.y - d2.x * d1.y;

			vec3 T(0.0f);
			if (std::abs(denom) > 1e-8f)
				T = (e1 * d2.y - e2 * d1.y) / denom;

			T = detail::OrthogonalizeTangent(T, N);

			v0.normal = v1.normal = v2.normal = N;
			v0.tangent = v1.tangent = v2.tangent = T;
		}*/
	}

	inline void CalculateNormalsAndTangentsSeamless(std::vector<Vertex>& _vertices)
	{
		constexpr float MERGE_EPSILON = 1e-5f;

		struct PosKey
		{
			int x, y, z;
			bool operator==(const PosKey& o) const { return x == o.x && y == o.y && z == o.z; }
		};
		struct PosKeyHash
		{
			size_t operator()(const PosKey& k) const
			{
				size_t h = std::hash<int>{}(k.x);
				h ^= std::hash<int>{}(k.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
				h ^= std::hash<int>{}(k.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
				return h;
			}
		};

		std::unordered_map<PosKey, uint32, PosKeyHash> slots;
		std::vector<uint32> slotOf(_vertices.size());
		for (size_t i = 0; i < _vertices.size(); ++i)
		{
			const vec3& p = _vertices[i].position;
			PosKey key{ (int)std::lround(p.x / MERGE_EPSILON),
			            (int)std::lround(p.y / MERGE_EPSILON),
			            (int)std::lround(p.z / MERGE_EPSILON) };
			slotOf[i] = slots.emplace(key, (uint32)slots.size()).first->second;
		}

		std::vector<vec3> nAcc(slots.size(), vec3(0.0f));
		std::vector<vec3> tAcc(slots.size(), vec3(0.0f));

		for (size_t i = 0; i + 2 < _vertices.size(); i += 3)
		{
			const Vertex& v0 = _vertices[i];
			const Vertex& v1 = _vertices[i + 1];
			const Vertex& v2 = _vertices[i + 2];

			vec3 e1 = v1.position - v0.position;
			vec3 e2 = v2.position - v0.position;
			vec3 faceNormal = cross(e1, e2);

			vec2 d1 = v1.uv - v0.uv;
			vec2 d2 = v2.uv - v0.uv;
			float denom = d1.x * d2.y - d2.x * d1.y;

			vec3 faceTangent(0.0f);
			if (std::abs(denom) >= 1e-6f)
				faceTangent = (e1 * d2.y - e2 * d1.y) / denom;

			for (size_t k = 0; k < 3; ++k)
			{
				nAcc[slotOf[i + k]] += faceNormal;
				tAcc[slotOf[i + k]] += faceTangent;
			}
		}
	}

	////////////////////////////////////////////
	//// TRIANGLE
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildTriangle()
	{
		std::vector<Vertex> vertices;

		// Vu depuis +Z : bas-gauche -> bas-droite -> haut = CCW
		detail::AddTriangle(vertices, { -0.5f, -0.5f, 0.0f },
		                              {  0.5f, -0.5f, 0.0f },
		                              {  0.0f,  0.5f, 0.0f });

		CalculateNormalsAndTangents(vertices);
		return vertices;
	}

	////////////////////////////////////////////
	//// PYRAMID
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildPyramid()
	{
		std::vector<Vertex> vertices;

		const vec3 A(-0.5f, -0.5f, -0.5f);
		const vec3 B( 0.5f, -0.5f, -0.5f);
		const vec3 C( 0.5f, -0.5f,  0.5f);
		const vec3 D(-0.5f, -0.5f,  0.5f);
		const vec3 T( 0.0f,  0.5f,  0.0f);

		detail::AddTriangle(vertices, B, A, T); // avant  (-Z)
		detail::AddTriangle(vertices, C, B, T); // droite (+X)
		detail::AddTriangle(vertices, D, C, T); // arriere(+Z)
		detail::AddTriangle(vertices, A, D, T); // gauche (-X)
		detail::AddQuad(vertices, A, B, C, D);  // base   (-Y)

		CalculateNormalsAndTangents(vertices);
		return vertices;
	}

	////////////////////////////////////////////
	//// CUBE
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildCube()
	{
		std::vector<Vertex> vertices;

		detail::AddQuad(vertices, { 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f}); // +X
		detail::AddQuad(vertices, {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f}); // -X
		detail::AddQuad(vertices, {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}); // +Y
		detail::AddQuad(vertices, {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f}); // -Y
		detail::AddQuad(vertices, {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}); // +Z
		detail::AddQuad(vertices, { 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}); // -Z

		CalculateNormalsAndTangents(vertices);
		return vertices;
	}

	////////////////////////////////////////////
	//// ICOSPHERE
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildIcosphere(int _subdivisions, float _radius = 0.5f)
	{
		_subdivisions = clamp(_subdivisions, 0, 6);

		using Tri = std::array<vec3, 3>;

		const float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

		std::vector<vec3> p = {
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

		// Subdivision : chaque triangle -> 4 triangles (CCW conserve)
		for (int s = 0; s < _subdivisions; ++s)
		{
			std::vector<Tri> next;
			next.reserve(tris.size() * 4);

			for (const Tri& tri : tris)
			{
				vec3 m0 = normalize((tri[0] + tri[1]) * 0.5f);
				vec3 m1 = normalize((tri[1] + tri[2]) * 0.5f);
				vec3 m2 = normalize((tri[2] + tri[0]) * 0.5f);

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
				const vec3& n = tri[k];
				u[k] = 0.5f + std::atan2(n.z, n.x) / (2.0f * pi<float>());
				v[k] = 0.5f - std::asin(clamp(n.y, -1.0f, 1.0f)) / pi<float>();
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
					if (!isPole[k] && u[k] < 0.5f)
						u[k] += 1.0f;

			for (int k = 0; k < 3; ++k)
				if (isPole[k])
					u[k] = (u[(k + 1) % 3] + u[(k + 2) % 3]) * 0.5f;

			for (int k = 0; k < 3; ++k)
			{
				const vec3& n = tri[k];
				vertices.push_back(detail::MakeVertex(n * _radius, { u[k], v[k] }));
			}
		}

		return vertices;
	}

	////////////////////////////////////////////
	//// UV SPHERE
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildUVSphere(uint32 _sliceCount, uint32 _stackCount, float _radius = 0.5f)
	{
		_sliceCount = std::max<uint32>(_sliceCount, 3);
		_stackCount = std::max<uint32>(_stackCount, 2);

		const float phiStep   = pi<float>() / (float)_stackCount;
		const float thetaStep = 2.0f * pi<float>() / (float)_sliceCount;

		auto sphereVertex = [&](uint32 i, uint32 j) -> Vertex
		{
			float phi   = (float)i * phiStep;
			float theta = (float)j * thetaStep;

			vec3 n(std::sin(phi) * std::cos(theta),
			            std::cos(phi),
			            std::sin(phi) * std::sin(theta));
			vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta));
			vec2 uv((float)j / (float)_sliceCount, (float)i / (float)_stackCount);

			return detail::MakeVertex(n * _radius, uv);
		};

		std::vector<Vertex> vertices;

		for (uint32 i = 0; i < _stackCount; ++i)
		{
			for (uint32 j = 0; j < _sliceCount; ++j)
			{
				Vertex a = sphereVertex(i,     j);
				Vertex b = sphereVertex(i,     j + 1);
				Vertex c = sphereVertex(i + 1, j);
				Vertex d = sphereVertex(i + 1, j + 1);

				float uMid = ((float)j + 0.5f) / (float)_sliceCount;

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

		return vertices;
	}

	////////////////////////////////////////////
	//// CIRCLE (disque)
	////////////////////////////////////////////

	// Ajoute un disque a _vertices. Top = normale +Y, sinon -Y. CCW vu de l'exterieur.
	inline void AddCircle(std::vector<Vertex>& _vertices, int _stackCount, float _posY, bool _isTop)
	{

		const float angle = 2.0f * pi<float>() / (float)_stackCount;

		Vertex center = detail::MakeVertex({ 0.0f, _posY, 0.0f }, { 0.5f, 0.5f });

		auto ringVertex = [&](int i) -> Vertex
		{
			float c = std::cos(angle * (float)i);
			float s = std::sin(angle * (float)i);
			return detail::MakeVertex({ c * 0.5f, _posY, s * 0.5f },
			                          { 0.5f + c * 0.5f, 0.5f + s * 0.5f });
		};

		for (int i = 0; i < _stackCount; ++i)
		{
			Vertex a = ringVertex(i);
			Vertex b = ringVertex(i + 1);

			if (_isTop) detail::PushTriangle(_vertices, center, b, a);
			else        detail::PushTriangle(_vertices, center, a, b);
		}
	}

	inline std::vector<Vertex> BuildCircle(int _stackCount, float _posY, bool _isTop)
	{
		std::vector<Vertex> vertices;
		AddCircle(vertices, std::max(_stackCount, 3), _posY, _isTop);
		return vertices;
	}

	////////////////////////////////////////////
	//// CYLINDER
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildCylinder(int _stackCount)
	{
		_stackCount = std::max(_stackCount, 3);

		std::vector<Vertex> vertices;
		AddCircle(vertices, _stackCount, -0.5f, false);
		AddCircle(vertices, _stackCount,  0.5f, true);

		const float angle = 2.0f * pi<float>() / (float)_stackCount;

		auto sideVertex = [&](int i, float y) -> Vertex
		{
			float u = (float)i / (float)_stackCount;
			float c = std::cos(angle * (float)i);
			float s = std::sin(angle * (float)i);
			return detail::MakeVertex({ c * 0.5f, y, s * 0.5f },
			                          { u, y < 0.0f ? 1.0f : 0.0f });
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

		return vertices;
	}

	////////////////////////////////////////////
	//// DONUT (tore)
	////////////////////////////////////////////

	inline std::vector<Vertex> BuildDonut(float _majorRadius, float _minorRadius, uint32 _majorSlices, uint32 _minorSlices)
	{
		_majorSlices = std::max<uint32>(_majorSlices, 3);
		_minorSlices = std::max<uint32>(_minorSlices, 3);

		const float twoPi = 2.0f * pi<float>();

		auto donutVertex = [&](uint32 i, uint32 j) -> Vertex
		{
			float u     = (float)i / (float)_majorSlices;
			float v     = (float)j / (float)_minorSlices;
			float theta = u * twoPi;
			float phi   = v * twoPi;

			float ring = _majorRadius + _minorRadius * std::cos(phi);

			vec3 p(ring * std::cos(theta),
			            _minorRadius * std::sin(phi),
			            ring * std::sin(theta));
			vec3 n(std::cos(phi) * std::cos(theta),
			            std::sin(phi),
			            std::cos(phi) * std::sin(theta));
			vec3 t(-std::sin(theta), 0.0f, std::cos(theta));

			return detail::MakeVertex(p, { u, v });
		};

		std::vector<Vertex> vertices;

		for (uint32 i = 0; i < _majorSlices; ++i)
		{
			for (uint32 j = 0; j < _minorSlices; ++j)
			{
				Vertex a = donutVertex(i,     j);
				Vertex b = donutVertex(i + 1, j);
				Vertex c = donutVertex(i,     j + 1);
				Vertex d = donutVertex(i + 1, j + 1);

				detail::PushTriangle(vertices, a, c, b);
				detail::PushTriangle(vertices, b, c, d);
			}
		}

		return vertices;
	}

	////////////////////////////////////////////
	//// OBJ LOADER
	////////////////////////////////////////////

	inline std::vector<Vertex> LoadGeometry(const std::string& _path)
	{
		std::ifstream file(_path);
		if (!file.is_open())
			return {};

		std::vector<vec3> rawPositions;
		std::vector<vec2> rawUVs;
		std::vector<Vertex>    vertices;

		auto resolve = [](int _i, size_t _count) -> int
		{
			if (_i > 0) return _i - 1;
			if (_i < 0) return (int)_count + _i;
			return -1;
		};

		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream ss(line);
			std::string header;
			ss >> header;

			if (header == "v")
			{
				vec3 p;
				ss >> p.x >> p.y >> p.z;
				rawPositions.push_back(p);
			}
			else if (header == "vt")
			{
				vec2 uv;
				ss >> uv.x >> uv.y;
				uv.y = 1.0f - uv.y;
				rawUVs.push_back(uv);
			}
			else if (header == "f")
			{
				struct Ref { int v = -1; int vt = -1; };
				std::vector<Ref> face;

				std::string token;
				while (ss >> token)
				{
					Ref r;
					size_t s1 = token.find('/');
					r.v = resolve(std::stoi(token.substr(0, s1)), rawPositions.size());

					if (s1 != std::string::npos)
					{
						size_t s2 = token.find('/', s1 + 1);
						std::string vt = token.substr(s1 + 1, s2 == std::string::npos ? std::string::npos : s2 - s1 - 1);
						if (!vt.empty())
							r.vt = resolve(std::stoi(vt), rawUVs.size());
					}
					face.push_back(r);
				}

				if (face.size() < 3)
					continue;

				bool valid = true;
				for (const Ref& r : face)
					if (r.v < 0 || r.v >= (int)rawPositions.size()) valid = false;
				if (!valid)
					continue;

				for (size_t k = 1; k + 1 < face.size(); ++k)
				{
					for (const Ref* r : { &face[0], &face[k], &face[k + 1] })
					{
						Vertex v;
						v.position = rawPositions[r->v];
						v.uv       = (r->vt >= 0 && r->vt < (int)rawUVs.size()) ? rawUVs[r->vt] : vec2(0.0f);
						v.color    = vec3((float)RANDOM::GetRandomInt32_t(0, 255));
						vertices.push_back(v);
					}
				}
			}
		}

		CalculateNormalsAndTangents(vertices);
		return vertices;
	}
}

#endif 