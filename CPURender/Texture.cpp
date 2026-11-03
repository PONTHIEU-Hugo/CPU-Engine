#include "Texture.h"
#include <cmath>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "core.h"
#include "stb_image.h"

void Texture::GenerateMipmaps()
{
	while (mips.back().width > 1 || mips.back().height > 1)
	{
		const MipLevel& src = mips.back();

		MipLevel dst;
		dst.width  = std::max(1, src.width  / 2);
		dst.height = std::max(1, src.height / 2);
		dst.pixels.resize((size_t)dst.width * dst.height);

		for (int y = 0; y < dst.height; ++y)
			for (int x = 0; x < dst.width; ++x)
			{
				int x0 = std::min(x * 2,     src.width  - 1);
				int x1 = std::min(x * 2 + 1, src.width  - 1);
				int y0 = std::min(y * 2,     src.height - 1);
				int y1 = std::min(y * 2 + 1, src.height - 1);

				uint32_t p[4] = {
					src.pixels[(size_t)y0 * src.width + x0],
					src.pixels[(size_t)y0 * src.width + x1],
					src.pixels[(size_t)y1 * src.width + x0],
					src.pixels[(size_t)y1 * src.width + x1],
				};

				uint32_t out = 0;
				for (int shift = 0; shift <= 24; shift += 8)
				{
					uint32_t sum = 0;
					for (int k = 0; k < 4; ++k)
						sum += (p[k] >> shift) & 0xFF;
					out |= ((sum + 2) / 4) << shift;
				}
				dst.pixels[(size_t)y * dst.width + x] = out;
			}

		mips.push_back(std::move(dst));
	}
}

bool Texture::Load(const char* path)
{
	int channels = 0;
	unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
	
	if (!data)
	{
		width = 0;
		height = 0;

		printf("Texture: impossible de charger %s : %s\n", path, stbi_failure_reason());
		return false;
	}

	mips.clear();
	mips.reserve(16);

	MipLevel base;
	base.width = width;
	base.height = height;
	base.pixels.resize((size_t)width * height);

	for (size_t i = 0; i < base.pixels.size(); ++i)
	{
		const unsigned char* p = data + i * 4;
		base.pixels[i] = (uint32_t(p[3]) << 24) | (uint32_t(p[0]) << 16) | (uint32_t(p[1]) << 8) | uint32_t(p[2]);
	}
	mips.push_back(std::move(base));

	stbi_image_free(data);
	GenerateMipmaps();
	return true;
}

glm::vec4 Texture::SampleLevel(int level, glm::vec2 uv) const
{
	const MipLevel& m = mips[level];

	float u = uv.x - std::floor(uv.x);
	float v = uv.y - std::floor(uv.y);

	int x = std::min((int)(u * m.width),  m.width  - 1);
	int y = std::min((int)(v * m.height), m.height - 1);

	uint32_t p = m.pixels[(size_t)y * m.width + x];
	return glm::vec4((float)((p >> 16) & 0xFF), (float)((p >> 8) & 0xFF),
					 (float)(p & 0xFF), (float)((p >> 24) & 0xFF));
}

glm::vec4 Texture::Sample(glm::vec2 uv, float lod) const
{
	if (!IsValid())
		return glm::vec4(255.0f, 0.0f, 255.0f, 255.0f);

	float maxLevel = (float)(mips.size() - 1);
	lod = glm::clamp(lod, 0.0f, maxLevel);

	int   l0 = (int)std::floor(lod);
	int   l1 = std::min(l0 + 1, (int)mips.size() - 1);
	float f  = lod - (float)l0;

	if (f < 1e-4f)
		return SampleLevel(l0, uv);

	return glm::mix(SampleLevel(l0, uv), SampleLevel(l1, uv), f);
}

float Texture::ComputeLod(glm::vec2 duvdx, glm::vec2 duvdy) const
{
	glm::vec2 size((float)width, (float)height);
	glm::vec2 dx = duvdx * size;
	glm::vec2 dy = duvdy * size;
	float footprint = std::max(glm::dot(dx, dx), glm::dot(dy, dy)); 
	return 0.5f * std::log2(std::max(footprint, 1e-8f));
}