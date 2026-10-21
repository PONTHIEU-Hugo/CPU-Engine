#ifndef FRUSTRUM_H_INCLUDED
#define FRUSTRUM_H_INCLUDED

#include <vector>
#include <glm/glm.hpp>
#include "Pipeline.h"

struct AABB
{
	glm::vec3 min{ 0.0f };
	glm::vec3 max{ 0.0f };
};

AABB ComputeAABB(const std::vector<Vertex>& vertices);

struct Frustum
{
	glm::vec4 planes[6];

	static Frustum FromMatrix(const glm::mat4& viewProj);

	bool Intersects(const AABB& worldBox) const;
};

#endif // !FRUSTRUM_H_INCLUDED