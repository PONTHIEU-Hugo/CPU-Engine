#pragma once

#include <vector>
#include "main.h"
#include "Pipeline.h"
//Students can change whetever they want in this file

union SDL_Event;

struct Engine 
{

	void init();

	void handleEvent(const SDL_Event& event);

	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

private:
	Pipeline m_pipeline;
	std::vector<Vertex> m_triangle; // vertex buffer
	float m_time = 0.0f;
};
