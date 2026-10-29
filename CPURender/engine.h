#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "main.h"
#include "Scene.h"

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
	Scene* m_scene = nullptr;
	
	Emitter  m_emitter{ 1000 };   // feu a l'echappement de la voiture (touche P : emission on/off)
	bool     m_pWasDown = false;  // front montant de la touche P (l'etat clavier est interroge en continu)
};

#endif
