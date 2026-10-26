#include "Clipping.h"

static float PlaneDistance(int plane, const glm::vec4& p)
{
	switch (plane)
	{
	case 0: return p.w + p.x; // gauche
	case 1: return p.w - p.x; // droite
	case 2: return p.w + p.y; // bas
	case 3: return p.w - p.y; // haut
	case 4: return p.w + p.z; // near
	default: return p.w - p.z; // far
	}
}

static Varyings LerpVaryings(const Varyings& a, const Varyings& b, float t)
{
	Varyings r;

	r.position = glm::mix(a.position, b.position, t);
	r.normal = glm::mix(a.normal, b.normal, t);
	r.color = glm::mix(a.color, b.color, t);
	r.uv = glm::mix(a.uv, b.uv, t);
	r.worldPos = glm::mix(a.worldPos, b.worldPos, t);
	r.lightPos = glm::mix(a.lightPos, b.lightPos, t);
	
	return r;
}

int ClipTriangle(const Varyings& a, const Varyings& b, const Varyings& c, Varyings out[12])
{
	bool allInside = true;

	for (int plane = 0; plane < 6; ++plane)
	{
		int outside = 0;
		outside += PlaneDistance(plane, a.position) < 0.0f;
		outside += PlaneDistance(plane, b.position) < 0.0f;
		outside += PlaneDistance(plane, c.position) < 0.0f;

		if (outside == 3)
			return 0;

		if (outside > 0)
			allInside = false;
	}

	if (allInside)
	{
		out[0] = a;
		out[1] = b;
		out[2] = c;

		return 3;
	}

	// Decoupage plan par plan
	Varyings buffers[2][12];
	buffers[0][0] = a;
	buffers[0][1] = b;
	buffers[0][2] = c;

	int count = 3;
	int current = 0;

	for (int plane = 0; plane < 6; ++plane)
	{
		const Varyings* in = buffers[current];
		Varyings* next = buffers[1 - current];
		int n = 0;

		for (int i = 0; i < count; ++i)
		{
			const Varyings& va = in[i];
			const Varyings& vb = in[(i + 1) % count];
			float da = PlaneDistance(plane, va.position);
			float db = PlaneDistance(plane, vb.position);

			if (da >= 0.0f)
				next[n++] = va;

			// On ajoute le point d'intersection si dépasse du plan
			if ((da >= 0.0f) != (db >= 0.0f))
				next[n++] = LerpVaryings(va, vb, da / (da - db));
		}

		count = n;
		current = 1 - current;

		if (count == 0)
			return 0;
	}

	for (int i = 0; i < count; ++i)
		out[i] = buffers[current][i];

	return count;
}
