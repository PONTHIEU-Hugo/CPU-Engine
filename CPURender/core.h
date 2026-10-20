#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include <random>
#include <glm/vec2.hpp>
#include <vector>

#include "main.h"
#include "Pipeline.h"

using namespace glm;

inline float getRandomFloat(float min, float max) 
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

#define FIRE_PALETTE_SIZE 37

static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
    0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
    0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
    0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
    0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
    0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
    0xEFEFC7, 0xFFFFFF
};

inline vec3 ToColor(uint32_t _color)
{
    uint8_t r = (_color >> 16) & 0xFF;
    uint8_t g = (_color >> 8) & 0xFF;
    uint8_t b = (_color >> 0)  & 0xFF;
    
    return vec3(r, g, b) / 255.0f;
}

#endif
