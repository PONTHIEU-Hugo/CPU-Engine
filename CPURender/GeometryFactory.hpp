#ifndef GEOMETRY_FACTORY_HPP_INCLUDED
#define GEOMETRY_FACTORY_HPP_INCLUDED

#include "core.h"

namespace GeometryFactory
{
    static std::vector<Vertex> LoadGeometry(std::string _path)
    {
    	std::vector<vec3> rawPositions;
    	std::vector<vec2> rawUVs;
    	
    	std::vector<Vertex>   vertices;
    	std::vector<uint32>   indices;

    	FILE* file = nullptr;
    	errno_t err = fopen_s(&file, _path.c_str(), "r");
    	if (err != 0 || file == nullptr)
    	{
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
    			rawPositions.push_back(pos);
    		}
    		else if (strcmp(lineHeader, "vt") == 0)
    		{
    			vec2 uv;
    			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
    			uv.y = 1.0f - uv.y;
    			rawUVs.push_back(uv);
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

    			for (int k = 0; k < 3; ++k)
    			{
    				Vertex v;
    				v.position = rawPositions[posIdx[k] - 1];
    				v.uv       = rawUVs.empty() ? vec2() : rawUVs[uvIdx[k] - 1];
    				v.color = ToColor(RANDOM::GetRandomInt32_t(0, 255));
    				indices.push_back((uint32)vertices.size());
    				vertices.push_back(v);
    			}
    		}
    	}

    	fclose(file);

		return vertices;
    }
}


#endif
