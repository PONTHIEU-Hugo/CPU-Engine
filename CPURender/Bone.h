#ifndef BONE_H_INCLUDED
#define BONE_H_INCLUDED

#include <assimp/anim.h>

#include "core.h"
#include <glm/gtc/quaternion.hpp>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    unsigned int m_NumPositions;
    unsigned int m_NumRotations;
    unsigned int m_NumScalings;
	
    glm::mat4 m_LocalTransform;
    std::string m_name;
    int m_ID;

public:

    Bone(const std::string& name, int ID, const aiNodeAnim* channel);
	
    void Update(float animationTime);
    
    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_name; }
    int GetBoneID() { return m_ID; }
    
    unsigned int GetPositionIndex(float animationTime);
    unsigned int GetRotationIndex(float animationTime);
    unsigned int GetScaleIndex(float animationTime);

private:

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    glm::mat4 InterpolatePosition(float animationTime);

    glm::mat4 InterpolateRotation(float animationTime);

    glm::mat4 InterpolateScaling(float animationTime);
	
};

#endif