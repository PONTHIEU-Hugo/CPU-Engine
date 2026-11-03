#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED

#include "core.h"
#include "Model.h"
#include "Bone.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;

    Animation(const std::string& animationPath, Model* model);

    ~Animation() { }

    Bone* FindBone(const std::string& name);
    
    float GetTicksPerSecond() { return m_TicksPerSecond; }
    float GetDuration() { return m_Duration;}
    const AssimpNodeData& GetRootNode() { return m_RootNode; }
    const std::unordered_map<std::string,BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
    void ReadMissingBones(const aiAnimation* animation, Model& model);

    void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
    
    float m_Duration;
    float m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
};

#endif