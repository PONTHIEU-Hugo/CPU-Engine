#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <cstdint>
#include <vector>
#include <array>
#include <glm/glm.hpp>

struct Texture
{
	int width = 0;
	int height = 0;
	std::vector<uint32_t> pixels;

	bool Load(const char* path);
	bool IsValid() const { return width > 0 && height > 0; }

	glm::vec3 Sample(glm::vec2 uv) const;
};

#endif // !TEXTURE_H_INCLUDED