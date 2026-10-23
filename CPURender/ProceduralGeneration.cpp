#include "ProceduralGeneration.h"
#include <iostream>

void Generation::Print()
{
	for (std::vector<int>& rVector : height)
	{
		for (int& rValue : rVector)
			std::cout << "[" << rValue << "]";
		std::cout << std::endl;
	}
}

Generation* Generation_Algorithm::DiamondSquare_HeighMap(size_t size)
{
	if (size < 2) return nullptr;

	size_t oddSize = size % 2 == 0 ? size + 1 : size;

	std::vector<int> column(oddSize, 0);
	std::vector<std::vector<int>> row;
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
