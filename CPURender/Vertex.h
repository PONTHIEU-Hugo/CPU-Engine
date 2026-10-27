#pragma once

#include <glm/glm.hpp>

// Sommet d'entree du pipeline (donnees d'un Mesh).
struct Vertex
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 normal   = { 0.0f, 0.0f, 0.0f };
	glm::vec3 color    = { 255.0f, 0.0f, 255.0f };
	glm::vec2 uv       = { 0.0f, 0.0f };
};
