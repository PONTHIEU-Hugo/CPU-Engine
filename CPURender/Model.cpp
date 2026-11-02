#ifndef MODEL_CPP_INCLUDED
#define MODEL_CPP_INCLUDED

#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

Mesh Model::Load(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights);

    Mesh result;
    if (!scene || !scene->mRootNode)
    {
        printf("Model: '%s' : %s", path.c_str(), importer.GetErrorString());
        return result;
    }

    bool anySkin = false;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        Mesh part = processMesh(scene->mMeshes[i], scene);
        anySkin |= part.IsSkinned();
        if (anySkin && part.skin.empty()) part.skin.resize(part.vertices.size());
        if (anySkin && result.skin.size() < result.vertices.size())
            result.skin.resize(result.vertices.size());
        result.vertices.insert(result.vertices.end(), part.vertices.begin(), part.vertices.end());
        result.skin.insert(result.skin.end(), part.skin.begin(), part.skin.end());
    }
    result.ComputeBounds();
    return result;
}

Mesh Model::processMesh(aiMesh* aMesh, const aiScene* scene)
{
    // 1) sommets indexés (temporaires)
    std::vector<Vertex> srcVerts(aMesh->mNumVertices);
    for (unsigned int i = 0; i < aMesh->mNumVertices; i++)
    {
        Vertex& v = srcVerts[i];
        v.position = AssimpGLMHelpers::GetGLMVec(aMesh->mVertices[i]);
        if (aMesh->HasNormals())
            v.normal = AssimpGLMHelpers::GetGLMVec(aMesh->mNormals[i]);
        if (aMesh->mTextureCoords[0])
            v.uv = { aMesh->mTextureCoords[0][i].x, 1.0f - aMesh->mTextureCoords[0][i].y };
    }

    std::vector<VertexSkin> srcSkin;
    if (aMesh->HasBones())
    {
        srcSkin.resize(aMesh->mNumVertices);
        ExtractBoneWeightForVertices(srcSkin, aMesh, scene);
    }

    Mesh result;
    result.vertices.reserve(aMesh->mNumFaces * 3);
    if (!srcSkin.empty()) result.skin.reserve(aMesh->mNumFaces * 3);

    for (unsigned int f = 0; f < aMesh->mNumFaces; f++)
    {
        const aiFace& face = aMesh->mFaces[f];
        if (face.mNumIndices != 3) continue; 
        for (unsigned int k = 0; k < 3; k++)
        {
            unsigned int idx = face.mIndices[k];
            result.vertices.push_back(srcVerts[idx]);
            if (!srcSkin.empty()) result.skin.push_back(srcSkin[idx]);
        }
    }

    result.ComputeBounds();
    return result;
}

void Model::SetVertexBoneData(VertexSkin& vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.boneIDs[i] < 0)
        {
            vertex.weights[i] = weight;
            vertex.boneIDs[i] = boneID;
            break;
        }
    }
}

void Model::ExtractBoneWeightForVertices(std::vector<VertexSkin>& vertices, aiMesh* mesh, const aiScene* scene)
{
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        }
        else
        {
            boneID = m_BoneInfoMap[boneName].id;
        }
        auto weights = mesh->mBones[boneIndex]->mWeights;
        unsigned int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (unsigned int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            unsigned int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId < vertices.size());
            SetVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

#endif
