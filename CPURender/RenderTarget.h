#ifndef RENDER_TARGET_H_INCLUDED
#define RENDER_TARGET_H_INCLUDED

#include "core.h"

struct RenderTarget
{
    int width  = 0;
    int height = 0;
    std::vector<float> depth;
    FinalFrameBuffer* color = nullptr; 

    RenderTarget() = default;
    RenderTarget(int w, int h, FinalFrameBuffer* c = nullptr)
        : width(w), height(h), depth((size_t)w * h, 1.0f), color(c) {}

    void ClearDepth() { std::fill(depth.begin(), depth.end(), 1.0f); }
};

#endif