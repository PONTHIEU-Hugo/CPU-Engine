#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <cstdint>
#include <vector>
#include <array>
#include <glm/glm.hpp>

struct MipLevel
{
	int width = 0;
	int height = 0;
	std::vector<uint32_t> pixels;
};

struct Texture
{
	int width = 0; 
	int height = 0;
	std::vector<MipLevel> mips; 

	bool Load(const char* path);
	bool IsValid() const { return width > 0 && height > 0 && !mips.empty(); }

	void GenerateMipmaps();
	float ComputeLod(glm::vec2 duvdx, glm::vec2 duvdy) const;

	glm::vec4 Sample(glm::vec2 uv) const { return Sample(uv, 0.0f); }
	glm::vec4 Sample(glm::vec2 uv, float lod) const;      
	glm::vec4 SampleLevel(int level, glm::vec2 uv) const; 
};

#endif // !TEXTURE_H_INCLUDED