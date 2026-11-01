#pragma once

#include <glm/glm.hpp>

// Sommet d'entree du pipeline (donnees d'un Mesh).
struct Vertex
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 normal   = { 0.0f, 0.0f, 0.0f };
	glm::vec4 color    = { 255.0f, 255.0f, 255.0f, 255.0f };
	glm::vec2 uv       = { 0.0f, 0.0f };
	glm::vec4 tangent  = glm::vec4(1, 0, 0, 1);
};

#define MAX_BONE_INFLUENCE 4

struct VertexSkin
{
	int   boneIDs[MAX_BONE_INFLUENCE] = { -1, -1, -1, -1 };
	float weights[MAX_BONE_INFLUENCE] = { 0.0f, 0.0f, 0.0f, 0.0f };
};