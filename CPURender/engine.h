#pragma once

#include <vector>

#include "ParticleSystem.h"
#include "main.h"
#include "Pipeline.h"
//Students can change whetever they want in this file

union SDL_Event;

struct Engine 
{

	void init();

	void Update(float _dt);
	void handleEvent(const SDL_Event& event);
	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

};
private:
	Emitter* m_emitter;
	Pipeline m_pipeline;
	std::vector<Vertex> m_triangle; // vertex buffer
	float m_time = 0.0f;

	glm::mat4 m_view{ 1.0f };
	glm::mat4 m_proj{ 1.0f };
};
