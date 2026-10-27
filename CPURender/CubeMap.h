#ifndef CUBEMAP_H_INCLUDED
#define CUBEMAP_H_INCLUDED

#include "Texture.h"

enum Face
{
	PlusX, NegX, PlusY, NegY, PlusZ, NegZ
};

struct CubeMap
{
	// ordre : +X, -X, +Y, -Y, +Z, -Z
	std::array<Texture, 6> faces;

	glm::vec3 Sample(glm::vec3 dir) const;
};

#endif // !CUBEMAP_H_INCLUDED

