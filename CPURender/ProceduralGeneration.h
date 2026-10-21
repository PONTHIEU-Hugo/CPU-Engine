#pragma once
#include "core.h"

struct Generation
{
	std::vector<std::vector<int>> height;
	glm::vec3 size;

	void Print();
};

namespace Generation_Algorithm
{
	// Size need to be 2**n or 2**n + 1
	Generation* DiamondSquare_HeighMap(size_t size);
}