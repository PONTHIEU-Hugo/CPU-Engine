#ifndef FRUSTRUM_H_INCLUDED
#define FRUSTRUM_H_INCLUDED

#include <glm/glm.hpp>
#include "AABB.h"

struct Frustum
{
	glm::vec4 planes[6];

	static Frustum FromMatrix(const glm::mat4& viewProj);

	bool Intersects(const AABB& worldBox) const;
};

#endif // !FRUSTRUM_H_INCLUDED
