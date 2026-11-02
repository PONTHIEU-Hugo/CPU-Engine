#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assimp/mesh.h>
#include <assimp/scene.h>

#include "assimp_glm_helpers.h"
#include "core.h"
#include "Mesh.h"
#include "Texture.h"

struct BoneInfo
{
    int id;
    glm::mat4 offset;

};

class Model 
{
private:
    std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

public:
    Mesh Load(const std::string& path);
    std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }   
    
    void SetVertexBoneDataToDefault(VertexSkin& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.boneIDs[i] = -1;
            vertex.weights[i] = 0.0f;
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    void SetVertexBoneData(VertexSkin& vertex, int boneID, float weight);
    void ExtractBoneWeightForVertices(std::vector<VertexSkin>& vertices, aiMesh* mesh, const aiScene* scene);
};

#endif