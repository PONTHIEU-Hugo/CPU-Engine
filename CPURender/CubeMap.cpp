#include "CubeMap.h"

glm::vec3 CubeMap::Sample(glm::vec3 dir) const
{
	glm::vec3 a = glm::abs(dir);
	int face;
	float mainAxis, faceU, faceV;

	if (a.x >= a.y && a.x >= a.z) // axe X dominant
	{
		mainAxis = a.x;
		if (dir.x > 0)
		{
			face = PlusX;
			faceU = -dir.z;
			faceV = -dir.y;
		}
		else
		{
			face = NegX;
			faceU = dir.z;
			faceV = -dir.y;
		}
	}
	else if (a.y >= a.z) // axe Y dominant
	{
		mainAxis = a.y;
		if (dir.y > 0)
		{
			face = PlusY;
			faceU = dir.x;
			faceV = dir.z;
		}

		else
		{
			face = NegY;
			faceU = dir.x;
			faceV = -dir.z;
		}
	}
	else  // axe Z dominant
	{
		mainAxis = a.z;
		if (dir.z > 0)
		{
			face = PlusZ;
			faceU = dir.x;
			faceV = -dir.y;
		}

		else
		{
			face = NegZ;
			faceU = -dir.x;
			faceV = -dir.y;
		}
	}

	if (mainAxis == 0.0f)	// direction nulle
		return glm::vec3(0.0f);

	glm::vec2 uv(0.5f * (faceU / mainAxis + 1.0f), 0.5f * (faceV / mainAxis + 1.0f));

	uv = glm::clamp(uv, glm::vec2(0.0f), glm::vec2(0.99999f));

	if ( !faces[face].IsValid() )
		return glm::vec3(0.0f);
	return faces[face].Sample(uv);
}
