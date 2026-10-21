#include "Frustum.h"

AABB ComputeAABB(const std::vector<Vertex>& vertices)
{
	AABB box;
	if (vertices.empty())
		return box;

	box.min = box.max = vertices[0].position;
	for (const Vertex& v : vertices)
	{
		box.min = glm::min(box.min, v.position);
		box.max = glm::max(box.max, v.position);
	}

	return box;
}

Frustum Frustum::FromMatrix(const glm::mat4& m)
{
	auto row = [&](int i) { return glm::vec4(m[0][i], m[1][i], m[2][i], m[3][i]); };
	glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

	Frustum f;
	f.planes[0] = r3 + r0; // gauche
	f.planes[1] = r3 - r0; // droite
	f.planes[2] = r3 + r1; // bas
	f.planes[3] = r3 - r1; // haut
	f.planes[4] = r3 + r2; // near
	f.planes[5] = r3 - r2; // far

	for (glm::vec4& p : f.planes)
		p /= glm::length(glm::vec3(p));

	return f;
}

bool Frustum::Intersects(const AABB& box) const
{
	for (const glm::vec4& p : planes)
	{
		glm::vec3 positive(
			p.x >= 0.0f ? box.max.x : box.min.x,
			p.y >= 0.0f ? box.max.y : box.min.y,
			p.z >= 0.0f ? box.max.z : box.min.z
		);

		if (glm::dot(glm::vec3(p), positive) + p.w < 0.0f)
			return false;
	}
	return true;
}
