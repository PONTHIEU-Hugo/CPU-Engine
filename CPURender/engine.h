#pragma once

#include <vector>

#include "main.h"
//Students can change whetever they want in this file

struct Vertex
{
	float x, y;
	uint32_t color;
};

struct Triangle
{
	float x, y;
	std::vector<Vertex> vertices;
	std::vector<uint8_t> indices;
};

union SDL_Event;

struct Engine {

	void init();

	void handleEvent(const SDL_Event& event);

	void Rasterization(FinalFrameBuffer& frameBufferOut, std::vector<Triangle>& triangles);
	void renderFrame(FinalFrameBuffer& frameBufferOut);

	void destroy();

};