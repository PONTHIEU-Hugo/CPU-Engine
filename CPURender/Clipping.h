#ifndef CLIPPING_H_INCLUDED
#define CLIPPING_H_INCLUDED

#include "Pipeline.h"

int ClipTriangle(const Varyings& a, const Varyings& b, const Varyings& c, Varyings out[12]);

#endif // !CLIPPING_H_INCLUDED