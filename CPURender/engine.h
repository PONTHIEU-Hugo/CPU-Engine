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
};

#endif
