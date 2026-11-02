#ifndef ANIMATOR_H_INCLUDED
#define ANIMATOR_H_INCLUDED

#include "core.h"
#include "Animation.h"
constexpr int MAX_BONES = 100;

class Animator
{	
public:
    Animator(Animation* Animation);
	
    void UpdateAnimation(float dt);
    void PlayAnimation(Animation* pAnimation);
    void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform);
	
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices; }
		
	uint64_t Version() const { return m_Version; }

private:
	uint64_t m_Version = 0;
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;	
};

#endif