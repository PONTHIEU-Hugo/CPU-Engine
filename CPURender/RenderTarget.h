#ifndef RENDER_TARGET_H_INCLUDED
#define RENDER_TARGET_H_INCLUDED

#include <algorithm>
#include <vector>
#include "main.h"

struct RenderTarget
{
    int width  = 0;
    int height = 0;
    std::vector<float> depth;
    FinalFrameBuffer* color = nullptr;
    std::vector<uint32_t> tint;

    RenderTarget() = default;
    RenderTarget(int w, int h, FinalFrameBuffer* c = nullptr)
        : width(w), height(h), depth((size_t)w * h, 1.0f), color(c) {}

    void ClearDepth() { std::fill(depth.begin(), depth.end(), 1.0f); }
    void EnableTint() { tint.assign((size_t)width * height, 0xFFFFFFu); }
    void ClearTint()  { std::fill(tint.begin(), tint.end(), 0xFFFFFFu); }
};

#endif
