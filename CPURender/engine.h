#pragma once

#include <vector>

#include "main.h"
#include "ParticleSystem.h"
//Students can change whetever they want in this file

union SDL_Event;

struct Engine {

	void init();

	void Update(float _dt);
	void handleEvent(const SDL_Event& event);
	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

	Emitter* m_emitter;
};