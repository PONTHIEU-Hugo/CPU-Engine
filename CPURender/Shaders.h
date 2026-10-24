#pragma once

#include "Pipeline.h"

// Catalogue des shaders : un shader = un nom.
// Pour en ajouter un : ecrire sa paire VS/PS dans Shaders.cpp et le declarer ici.
extern const Shader DepthShader;
extern const Shader BasicShader;
extern const Shader TextureShader;

// Texture (ou couleur des sommets) eclairee par la lumiere directionnelle des Uniforms.
// Necessite des normales dans les sommets (voir Pipeline::ComputeNormals).
extern const Shader LambertShader;
