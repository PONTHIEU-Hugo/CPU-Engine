#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include <cstdint>
#include <limits>
#include <random>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Chrono.h"

namespace RANDOM
{
    inline std::mt19937& Generator()
    {
        static std::mt19937 gen{ std::random_device{}() };
        return gen;
    }

    inline float GetRandomFloat(float min, float max)
    {
        std::uniform_real_distribution<float> dis(min, max);
        return dis(Generator());
    }

    inline int GetRandomInt(int min, int max)
    {
        std::uniform_int_distribution<int> dis(min, max);
        return dis(Generator());
    }

    // Entier 32 bits sur toute la plage.
    inline int32_t GetRandomInt32_t()
    {
        std::uniform_int_distribution<int32_t> dis(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
        return dis(Generator());
    }
}

// 0xRRGGBB -> vec3 (0-255)
inline glm::vec4 ToColor(uint32_t _color)
{
    uint8_t r = (_color >> 16) & 0xFF;
    uint8_t g = (_color >> 8) & 0xFF;
    uint8_t b = (_color >> 0) & 0xFF;
    uint8_t a = (_color >> 24) & 0xFF;
    return glm::vec4(r, g, b, a);
}

#endif
