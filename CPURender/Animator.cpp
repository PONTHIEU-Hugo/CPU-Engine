#ifndef ANIMATOR_CPP_INCLUDED
#define ANIMATOR_CPP_INCLUDED

#include "Animator.h"

Animator::Animator(Animation* animation)
{
    m_CurrentTime = 0.0f;
    m_DeltaTime = 0.0f;
    m_CurrentAnimation = animation;
    m_FinalBoneMatrices.assign(MAX_BONES, glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt)
{
    
    m_DeltaTime = dt;
    if (m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        ++m_Version;
    }
}

void Animator::PlayAnimation(Animation* pAnimation)
{
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform)
{
    glm::mat4 nodeTransform = node->transformation;

    if (Bone* bone = m_CurrentAnimation->FindBone(node->name))
    {
        bone->Update(m_CurrentTime);
        nodeTransform = bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    const auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    auto it = boneInfoMap.find(node->name);
    if (it != boneInfoMap.end() && it->second.id < (int)m_FinalBoneMatrices.size())
        m_FinalBoneMatrices[it->second.id] = globalTransformation * it->second.offset;

    for (const AssimpNodeData& child : node->children)
        CalculateBoneTransform(&child, globalTransformation);
}

#endif
