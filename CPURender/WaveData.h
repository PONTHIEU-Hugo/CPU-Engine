#pragma once
#include "core.h"

struct WaveData
{
	float k = 0.f;							// difference of sin between here and next => in one lambda
	float angularFrequency = 1.f;			// time for a vertex to do a cycle of movement
	float amplitude = 0.f;					// highness of wave
	float sharpness = 1.f;					// sharpness of waves
	glm::vec2 direction = { 0.f, 0.f };		// direction where wave progress
};