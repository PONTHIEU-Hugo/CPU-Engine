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

// Texture (ou couleur des sommets) eclairee par la lumiere directionnelle des Uniforms.
// Necessite des normales dans les sommets (voir GeometryFactory::CalculateNormals). 
extern const Shader LambertShader;
extern const Shader RefractionShader;

// Lambert + ombre portee (lit Uniforms::shadowMap / lightVP / bias).
extern const Shader ShadowLambertShader;

// Rendu d'animation des vagues d'eau
// Rendu basique de vagues arrondis
extern const Shader SinWavesShader;
// Rendu plus réaliste utilisant les vagues de Gerstner
extern const Shader GerstnerWavesShader;