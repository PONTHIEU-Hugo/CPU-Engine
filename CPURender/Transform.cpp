#ifndef TRANSFORM_CPP_INCLUDED
#define TRANSFORM_CPP_INCLUDED

#include "Transform.h"
#include <glm/gtc/quaternion.hpp>

void Transform::UpdateMatrix()
{
    glm::quat<float> gQuat = glm::quat<float>(rotation.w, rotation.x, rotation.y, rotation.z);
    
    glm::mat4 mScale = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 mRot = glm::mat4_cast(gQuat);
    glm::mat4 mTrans = glm::translate(glm::mat4(1.0f), position);
    
    wolrd = mTrans * mRot * mScale;
    local = wolrd;
}

void Transform::SetPosition(glm::vec3 _position)
{
    position = _position;
    isDirty = true;
}

void Transform::SetScale(glm::vec3 _scale)
{
    scale = _scale;
    isDirty = true;
}

void Transform::SetRotation(glm::vec4 _rotation)
{
    rotation = _rotation;
    isDirty = true;
}

void Transform::SetYawPitchRoll(glm::vec3 _euler)
{
    rotation = quat<float>(euler);
    isDirty = true;
}

void Transform::Move(glm::vec3 _dir)
{
    position += _dir;
    isDirty = true;
}

void Transform::LookAt(glm::vec3 _target)
{
    glm::vec3 forwardDir = glm::normalize((_target - position));
    glm::vec3 rightDir = glm::normalize(glm::cross({0.0f, 1.0f, 0.0f}, forwardDir));
    glm::vec3 upDir = glm::normalize(glm::cross(forwardDir, rightDir));

    this->forward = forwardDir;
    this->right = rightDir;
    this->up = upDir;
    
    isDirty = true;
}

void Transform::OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius)
{
}



#endif
