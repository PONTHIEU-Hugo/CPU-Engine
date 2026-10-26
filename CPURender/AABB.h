#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Vertex.h"

struct AABB
{
	glm::vec3 min{ 0.0f };
	glm::vec3 max{ 0.0f };
};

// Boite englobante d'une liste de sommets (espace local).
AABB ComputeAABB(const std::vector<Vertex>& vertices);

// Boite englobante (alignee sur les axes) d'une AABB transformee par une matrice.
AABB TransformAABB(const AABB& box, const glm::mat4& matrix);
