#include "ProceduralGeneration.h"
#include <iostream>

void Generation::Print()
{
	for (std::vector<double>& rVector : height)
	{
		for (double& rValue : rVector)
			std::cout << "[" << rValue << "]";
		std::cout << std::endl;
	}
}

// DIAMOND SQUARE

Generation* Generation_Algorithm::DiamondSquare_HeighMap(size_t size)
{
	if (size < 2) return nullptr;

	size_t oddSize = size % 2 == 0 ? size + 1 : size;

	std::vector<double> column(oddSize, 0);
	std::vector<std::vector<double>> row;
	for (size_t i = 0; i < oddSize; i++)
		row.push_back(column);

	float topL, topR, bottomL, bottomR = 0;

	// Initialize
	row[0][0] =						RANDOM::GetRandomInt(0, 255);
	row[0][oddSize - 1] =			RANDOM::GetRandomInt(0, 255);
	row[oddSize - 1][0] =			RANDOM::GetRandomInt(0, 255);
	row[oddSize - 1][oddSize - 1] = RANDOM::GetRandomInt(0, 255);

	int step = oddSize;
	while (step > 1)
	{
		int half = step * 0.5f;

		// Diamond
		for (int x = half; x < oddSize; x += step)
		{
			for (int y = half; y < oddSize; y += step)
			{
				float value = 0.f;
				int added = 0;

				if (y - half >= 0)
				{
					value = row[y - half][x];
					added++;
				}
				if (x - half >= 0)
				{
					value = row[y][x - half];
					added++;
				}
				if (x + half <= oddSize)
				{
					value = row[y][x + half];
					added++;
				}
				if (y + half <= oddSize)
				{
					value = row[y + half][x];
					added++;
				}

				value /= added;
				value += RANDOM::GetRandomInt(0, 127);
				row[y][x] = value;
			}
		}

		// Square
		for (int x = 0; x < oddSize; x += half)
		{
			for (int y = (x % 2 == 0 ? half : 0); y < oddSize; y += step)
			{
				float value = 0.f;
				int added = 0;

				if (y - half >= 0)
				{
					value = row[y - half][x];
					added++;
				}
				if (x - half >= 0)
				{
					value = row[y][x - half];
					added++;
				}
				if (x + half < oddSize)
				{
					value = row[y][x + half];
					added++;
				}
				if (y + half < oddSize)
				{
					value = row[y + half][x];
					added++;
				}

				value /= added;
				value += RANDOM::GetRandomInt(0, 127);
				row[y][x] = value;
			}
		}

		step *= 0.5f;
	}
	return new Generation{ row, { oddSize, oddSize, 1 } };
}

/// PERLIN

std::vector<int> Generation_Algorithm::PerlinNoise::Hash(std::vector<int>& p, glm::ivec3 pos)
{
	int aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = p[p[p[pos.x] + pos.y] + pos.z];
	aba = p[p[p[pos.x] + pos.y + 1] + pos.z];
	aab = p[p[p[pos.x] + pos.y] + pos.z + 1];
	abb = p[p[p[pos.x] + pos.y + 1] + pos.z  +1];
	baa = p[p[p[pos.x + 1] + pos.y] + pos.z];
	bba = p[p[p[pos.x + 1] + pos.y + 1] + pos.z];
	bab = p[p[p[pos.x + 1] + pos.y] + pos.z + 1];
	bbb = p[p[p[pos.x + 1] + pos.y + 1] + pos.z + 1];

	return std::vector<int>{
		aaa,
		aba,
		aab,
		abb,
		baa,
		bba,
		bab,
		bbb
	};
}

double Generation_Algorithm::PerlinNoise::Fade(double d)
{
	return d * d * d * (d * (d * 6.0 - 15.0) + 10.0);
}

double Generation_Algorithm::PerlinNoise::Grad(int hash, glm::dvec3 pos)
{
	switch (hash & 0xF)
	{
	case 0x0: return  pos.x + pos.y;
	case 0x1: return -pos.x + pos.y;
	case 0x2: return  pos.x - pos.y;
	case 0x3: return -pos.x - pos.y;
	case 0x4: return  pos.x + pos.z;
	case 0x5: return -pos.x + pos.z;
	case 0x6: return  pos.x - pos.z;
	case 0x7: return -pos.x - pos.z;
	case 0x8: return  pos.y + pos.z;
	case 0x9: return -pos.y + pos.z;
	case 0xA: return  pos.y - pos.z;
	case 0xB: return -pos.y - pos.z;
	case 0xC: return  pos.y + pos.x;
	case 0xD: return -pos.y + pos.z;
	case 0xE: return  pos.y - pos.x;
	case 0xF: return -pos.y - pos.z;
	default: return 0; // never happens
	}
}

double Generation_Algorithm::PerlinNoise::Lerp(double begin, double end, double x)
{
	return begin + x * (end - begin);
}

double Generation_Algorithm::PerlinNoise::PerlinFor(std::vector<int>& doublePermutation, glm::ivec2 size, glm::dvec3 pos)
{
	glm::ivec3 posI = { (int)pos.x, (int)pos.y, (int)pos.z };
	glm::dvec3 posD = { pos.x - posI.x, pos.y - posI.y, pos.z - posI.z };
	glm::dvec3 fadePos = { Fade(posD.x), Fade(posD.y), Fade(posD.z) };

	posI = posI & 255;
	std::vector<int> hash = Hash(doublePermutation, posI);

	double x1, x2, y1, y2, z1, z2;
	x1 = Lerp(Grad(hash[0], posD),
		Grad(hash[4], posD - glm::dvec3{ 1.0, 0.0, 0.0 }),
		fadePos.x);
	x2 = Lerp(Grad(hash[1], posD - glm::dvec3{ 0.0, 1.0, 0.0 }),
		Grad(hash[5], posD - glm::dvec3{ 1.0, 1.0, 0.0 }),
		fadePos.x);
	y1 = Lerp(x1, x2, fadePos.y);

	x1 = Lerp(Grad(hash[2], posD - glm::dvec3{ 0.0, 0.0, 1.0 }),
		Grad(hash[6], posD - glm::dvec3{ 1.0, 0.0, 1.0 }),
		fadePos.x);
	x2 = Lerp(Grad(hash[3], posD - glm::dvec3{ 0.0, 1.0, 1.0 }),
		Grad(hash[7], posD - glm::dvec3{ 1.0, 1.0, 1.0 }),
		fadePos.x);
	y2 = Lerp(x1, x2, fadePos.y);
	double result = Lerp(y1, y2, fadePos.z);
	return result;
}

Generation* Generation_Algorithm::Perlin(glm::ivec2 size, double scale)
{
	std::vector<int> doublePermutation(PerlinNoise::permutationSize * 2);
	for (int i = 0; i < PerlinNoise::permutationSize * 2; i++)
		doublePermutation[i] = PerlinNoise::permutation[i % PerlinNoise::permutationSize];

	Generation* pGeneration = new Generation();
	pGeneration->height.resize(size.y);
	for (size_t i = 0; i < size.y; i++)
		pGeneration->height[i].resize(size.x);

	for (int i = 0; i < size.y; i++)
	{
		for (int j = 0; j < size.x; j++)
			pGeneration->height[i][j] = PerlinNoise::PerlinFor(doublePermutation, size, glm::dvec3{ j, i, 0.f } * scale);
	}
	pGeneration->size = glm::vec3(size.x, size.y, 1.f);

	return pGeneration;
}

Generation* Generation_Algorithm::PerlinOctave(glm::ivec2 size, double scale, int octave, double persistence)
{
	std::vector<int> doublePermutation(PerlinNoise::permutationSize * 2);
	for (int i = 0; i < PerlinNoise::permutationSize * 2; i++)
		doublePermutation[i] = PerlinNoise::permutation[i % PerlinNoise::permutationSize];

	Generation* pGeneration = new Generation();
	pGeneration->height.resize(size.y);
	for (size_t i = 0; i < size.y; i++)
		pGeneration->height[i].resize(size.x);

	for (int i = 0; i < size.y; i++)
	{
		for (int j = 0; j < size.x; j++)
		{
			double frequency = 1.0;
			double amplitude = 1.0;
			double total = 0.0;
			double maxValue = 0.0;
			for (int currentOctave = 0; currentOctave < octave; currentOctave++)
			{
				total += PerlinNoise::PerlinFor(doublePermutation, size, glm::dvec3{ j, i, 0.f } * frequency * scale) * amplitude;

				maxValue += amplitude;
				amplitude *= persistence;
				frequency *= 2.0;
			}
			
			pGeneration->height[i][j] = total / maxValue;
		}
	}
	pGeneration->size = glm::vec3(size.x, size.y, 1.f);

	return pGeneration;	
}