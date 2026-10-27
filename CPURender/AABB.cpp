#include "AABB.h"

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

AABB TransformAABB(const AABB& box, const glm::mat4& matrix)
{
	AABB result;
	result.min = glm::vec3(1e30f);
	result.max = glm::vec3(-1e30f);

	for (int i = 0; i < 8; ++i)
	{
		glm::vec3 corner((i & 1) ? box.max.x : box.min.x,
		                 (i & 2) ? box.max.y : box.min.y,
		                 (i & 4) ? box.max.z : box.min.z);
		glm::vec3 world = glm::vec3(matrix * glm::vec4(corner, 1.0f));
		result.min = glm::min(result.min, world);
		result.max = glm::max(result.max, world);
	}

	return result;
}
