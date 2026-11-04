#pragma once

#include "Pipeline.h"

// Catalogue des shaders : un shader = un nom.
// Pour en ajouter un : ecrire sa paire VS/PS dans Shaders.cpp et le declarer ici.

// Profondeur seule (passe d'ombre) : pas de pixel shader.
extern const Shader DepthShader;
extern const Shader TintShader;

extern const Shader BasicShader;
extern const Shader TextureShader;
extern const Shader ParticleShader;
extern const Shader SkyboxShader;

// Texture (ou couleur des sommets) eclairee par la lumiere directionnelle des Uniforms.
// Necessite des normales dans les sommets (voir GeometryFactory::CalculateNormals). 
extern const Shader LambertShader;
extern const Shader RefractionShader;

// Lambert + ombre portee (lit Uniforms::shadowMap / lightVP / bias).
extern const Shader ShadowLambertShader;
