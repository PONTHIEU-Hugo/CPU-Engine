#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "main.h"
#include "Scene.h"
#include "Renderer.h"
#include "Texture.h"
#include "ParticleSystem.h"
#include "TerrainGenerator.h"

union SDL_Event;

// Ne s'occupe plus du rendu, se contente de l'appeler
struct Engine
{
	void init();

	void Update();
	void handleEvent();
	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

private:
	// Ressources : possedees ici, referencees (par pointeur) par la scene.
	Mesh     m_cubeMesh;
	Mesh     m_carMesh;
	Mesh     m_wheelAvMesh;
	Mesh     m_wheelArMesh;
	Mesh     m_groundMesh;
	Texture  m_carTexture;

	Material m_defaultMaterial;
	Material m_carMaterial;
	Material m_groundMaterial;

	std::vector<Transform*> m_transforms;

	Scene    m_scene;
	Renderer m_renderer;

	ObjectId m_car = 0;

	Emitter  m_emitter{ 1000 };   // feu a l'echappement de la voiture (touche P : emission on/off)
	bool     m_pWasDown = false;  // front montant de la touche P (l'etat clavier est interroge en continu)

	TerrainGenerator m_Terrain;
};

#endif
