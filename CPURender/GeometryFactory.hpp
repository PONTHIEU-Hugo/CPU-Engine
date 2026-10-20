#ifndef GEOMETRY_FACTORY_HPP_INCLUDED
#define GEOMETRY_FACTORY_HPP_INCLUDED

#include "core.h"

namespace GeometryFactory
{
    static std::vector<Vertex> LoadGeometry(std::string _path)
    {
    	
    	std::vector<vec3> rawPositions;
    	std::vector<Vertex>   vertices;

    	FILE* file = nullptr;
    	errno_t err = fopen_s(&file, _path.c_str(), "r");
    	if (err != 0 || file == nullptr)
    	{
    		// std::cout << "Impossible to open the file: " << _path << '\n';
    		return {};
    	}

    	while (1)
    	{
    		char lineHeader[128];
    		int res = fscanf_s(file, "%s", lineHeader, (unsigned)_countof(lineHeader));
    		if (res == EOF)
    			break;

    		if (strcmp(lineHeader, "v") == 0)
    		{
    			vec3 pos;
    			fscanf_s(file, "%f %f %f\n", &pos.x, &pos.y, &pos.z);
    			pos.z = -pos.z;
    			rawPositions.push_back(pos);
    		}
    		else if (strcmp(lineHeader, "f") == 0)
    		{
    			unsigned int posIdx[3], uvIdx[3], normalIdx[3];
    			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&posIdx[0], &uvIdx[0], &normalIdx[0],
					&posIdx[1], &uvIdx[1], &normalIdx[1],
					&posIdx[2], &uvIdx[2], &normalIdx[2]);

    			if (matches != 9)
    			{
    				printf("File can't be read by our simple parser\n");
    				fclose(file);
    				return {};
    			}

    			int order[3] = {0, 2, 1};
    			for (int k = 0; k < 3; ++k)
    			{
    				int idx = order[k];
    				Vertex v;
    				v.position = rawPositions[posIdx[idx] - 1];
    				v.color = ToColor(getRandomFloat(0, 255));
    				vertices.push_back(v);
    			}
    		}
    	}

    	fclose(file);

		return vertices;
    }
}


#endif
