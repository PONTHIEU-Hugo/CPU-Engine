#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include <vector>
#include "Vertex.h"
#include "AABB.h"

// Donnees geometriques partagees et immuables apres chargement.
// Un meme Mesh peut etre affiche par plusieurs RenderObject (instances).
struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<VertexSkin> skin;
	AABB localBounds;               

	bool IsSkinned() const { return !skin.empty(); }
	void ComputeBounds() { localBounds = ComputeAABB(vertices); }
};

#endif // !MESH_H_INCLUDED