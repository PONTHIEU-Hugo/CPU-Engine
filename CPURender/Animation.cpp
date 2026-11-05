#ifndef ANIMATION_CPP_INCLUDED
#define ANIMATION_CPP_INCLUDED

#include "Animation.h"
#include "assimp_glm_helpers.h"
#include <assimp/postprocess.h>
#include <filesystem>

static const aiNode* FindMeshNode(const aiNode* n)
{
    if (n->mNumMeshes > 0) return n;
    for (unsigned i = 0; i < n->mNumChildren; i++)
        if (const aiNode* r = FindMeshNode(n->mChildren[i])) return r;
    return nullptr;
}

static void BindMeshToBone(Mesh& mesh, int boneID)
{
    mesh.skin.assign(mesh.vertices.size(), VertexSkin{});
    for (auto& s : mesh.skin)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) { s.boneIDs[i] = -1; s.weights[i] = 0.0f; }
        s.boneIDs[0] = boneID;
        s.weights[0] = 1.0f;
    }
}

Animation::Animation(const std::string& animationPath, Model* model, Mesh* staticMesh)
{
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);

    if (!scene || !scene->mRootNode)
    {
        printf("Assimp: impossible de charger '%s' : %s",
                animationPath.c_str(), importer.GetErrorString());
        return;
    }
    if (scene->mNumAnimations == 0)
    {
        printf("Assimp: '%s' ne contient aucune animation", animationPath.c_str());
        return;
    }

    auto animation = scene->mAnimations[0];
    m_Duration       = (float)animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond != 0.0 ? (float)animation->mTicksPerSecond : 25.0f;
    ReadHeirarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);

    if (staticMesh && !staticMesh->IsSkinned())
    {
        const aiNode* meshNode = FindMeshNode(scene->mRootNode);
        if (!meshNode) { printf("Animation: aucun nœud portant un mesh\n"); return; }

        auto& boneInfoMap = model->GetBoneInfoMap();
        int&  boneCount   = model->GetBoneCount();
        std::string name  = meshNode->mName.data;

        if (boneInfoMap.find(name) == boneInfoMap.end())
        {
            BoneInfo info;
            info.id = boneCount++;
            info.offset = glm::mat4(1.0f);
            boneInfoMap[name] = info;
            m_BoneInfoMap = boneInfoMap;
        }
        BindMeshToBone(*staticMesh, boneInfoMap[name].id);
    }
}

Bone* Animation::FindBone(const std::string& name)
{
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
        [&](const Bone& Bone)
        {
            return Bone.GetBoneName() == name;
        }
    );
    if (iter == m_Bones.end()) return nullptr;
    else return &(*iter);
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
    int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

    
    //reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        printf("canal: %s\n", channel->mNodeName.data);
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo info;
            info.id = boneCount++;
            info.offset = glm::mat4(1.0f);
            boneInfoMap[boneName] = info;
        }
        m_Bones.push_back(Bone(channel->mNodeName.data,
            boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
{
    dest.name = src->mName.data;
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (unsigned int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
#endif
