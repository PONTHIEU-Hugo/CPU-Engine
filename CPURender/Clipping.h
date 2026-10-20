#ifndef CLIPPING_H_INCLUDED
#define CLIPPING_H_INCLUDED

#include "Pipeline.h"

static float PlaneDistance(int plane, const glm::vec4& p);
static Varyings LerpVaryings(const Varyings& a, const Varyings& b, float t);

int ClipTriangle(const Varyings& a, const Varyings& b, const Varyings& c, Varyings out[12]);

#endif // !CLIPPING_H_INCLUDED