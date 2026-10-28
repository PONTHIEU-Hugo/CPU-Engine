#include "Texture.h"
#include <cmath>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "core.h"
#include "stb_image.h"

bool Texture::Load(const char* path)
{
	int channels = 0;
	unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
	
	if (!data)
	{
		width = 0;
		height = 0;
		pixels.clear();

		printf("Texture: impossible de charger %s : %s\n", path, stbi_failure_reason());
		return false;
	}

	pixels.resize((size_t)width * height);

	for (size_t i = 0; i < pixels.size(); ++i)
	{
		const unsigned char* p = data + i * 4;
		pixels[i] = (uint32_t(p[3]) << 24) | (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8)  | uint32_t(p[2]);
	}

	stbi_image_free(data);

	return true;
}

glm::vec4 Texture::Sample(glm::vec2 uv) const
{
	if (IsValid() == false)
		return glm::vec4(255.0f, 0.0f, 255.0f, 255.0f);

	float u = uv.x - std::floor(uv.x);
	float v = uv.y - std::floor(uv.y);

	int x = (int)(u * width); 
	int y = (int)(v * height);

	if (x >= width)
		x = width - 1;

	if (y >= height)
		y = height - 1;

	uint32_t p = pixels[(size_t)y * width + x];

	return glm::vec4((float)((p >> 16) & 0xFF), (float)((p >> 8) & 0xFF),(float)(p & 0xFF), (float)((p >> 24) & 0xFF));
}