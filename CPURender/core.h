#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include <random>
#include <glm/vec2.hpp>
#include <vector>

#include "main.h"

using namespace glm;

struct Vertex
{
    float x, y;
};

struct Triangle
{
    vec2 pos;
    std::vector<Vertex> vertices;
    int32_t color;    
};

inline float getRandomFloat(float min, float max) 
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

inline void DrawHorizontalLine(FinalFrameBuffer& frameBufferOut, int32_t startX, int32_t endX, uint32_t y, uint32_t color)
{
    for (int i = 0; i < abs(endX - startX); i++)
    {
        frameBufferOut.put_pixel(startX + i, y, color);
    }
}

inline void Rasterization(FinalFrameBuffer& frameBufferOut, Triangle& triangles)
{
    vec2 p1 = {triangles.vertices[0].x , triangles.vertices[0].y};
    vec2 p2 = {triangles.vertices[1].x , triangles.vertices[1].y};
    vec2 p3 = {triangles.vertices[2].x , triangles.vertices[2].y};
    
    if (p1.y > p2.y) std::swap(p1, p2);
    if (p1.y > p3.y) std::swap(p1, p3);
    if (p2.y > p3.y) std::swap(p2, p3);

    if (p1.y == p3.y) return;
    
    for (int y = (int)p1.y; y < (int)p2.y; ++y) 
    {
        float t1 = ((float)y - p1.y) / (p3.y - p1.y);
        float t2 = ((float)y - p1.y) / (p2.y - p1.y);
    
        int32_t xStart = int32_t(p1.x + (p3.x - p1.x) * t1);
        int32_t xEnd   = int32_t(p1.x + (p2.x - p1.x) * t2);
    
        if ( xStart > xEnd) std::swap(xStart, xEnd);
        DrawHorizontalLine(frameBufferOut, xStart + triangles.pos.x, xEnd + triangles.pos.x, y + triangles.pos.y, triangles.color);
    }

    for (int y = (int)p2.y; y < (int)p3.y; ++y) 
    {
        float t1 = ((float)y - p1.y) / (p3.y - p1.y);
        float t2 = ((float)y - p2.y) / (p3.y - p2.y);
    
        int32_t xStart = int32_t(p1.x + (p3.x - p1.x) * t1);
        int32_t xEnd   = int32_t(p2.x + (p3.x - p2.x) * t2);
    
        if ( xStart > xEnd) std::swap(xStart, xEnd);
        DrawHorizontalLine(frameBufferOut, xStart + triangles.pos.x, xEnd + triangles.pos.x, y + triangles.pos.y, triangles.color);
    }
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

#endif
