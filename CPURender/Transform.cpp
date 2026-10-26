#ifndef TRANSFORM_CPP_INCLUDED
#define TRANSFORM_CPP_INCLUDED

#include "Transform.h"

#include "ShadowMap.h"

void Transform::UpdateMatrix() const
{
    bool changed = isDirty;

    if (isDirty)
    {
        local = glm::translate(glm::mat4(1.0f), position)
              * glm::mat4_cast(rotation)
              * glm::scale(glm::mat4(1.0f), scale);
        isDirty = false;
    }

    glm::mat4 newWorld = parent ? parent->GetWorldMatrix() * local : local;

    if (changed || newWorld != world)
    {
        world = newWorld;
        ShadowMap::GetInstance().SetDirty();
    }
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

void Transform::SetRotation(glm::quat _rotation)
{
    rotation = _rotation;
    isDirty = true;
}

void Transform::SetYawPitchRoll(glm::vec3 _euler)
{
    rotation = glm::quat(_euler);
    isDirty = true;
}

void Transform::AddYPR(const glm::vec3& ypr)
{
    // ypr.x = yaw (axe Y monde), ypr.y = pitch (axe X local), ypr.z = roll (axe Z local)
    const glm::vec3 axisY(0.0f, 1.0f, 0.0f);
    const glm::vec3 axisX(1.0f, 0.0f, 0.0f);
    const glm::vec3 axisZ(0.0f, 0.0f, 1.0f);

    if (ypr.x != 0.0f)
        rotation = glm::angleAxis(ypr.x, axisY) * rotation;

    if (ypr.y != 0.0f) // pitch : axe local
        rotation = rotation * glm::angleAxis(ypr.y, axisX);

    if (ypr.z != 0.0f) // roll : axe local
        rotation = rotation * glm::angleAxis(ypr.z, axisZ);

    rotation = glm::normalize(rotation);
    isDirty = true;
}

void Transform::AddScale(const glm::vec3& _scale)
{
    scale += _scale;
    isDirty = true;
}

void Transform::AddScale(const float& _scale)
{
    scale += _scale;
    isDirty = true;
}

void Transform::Move(glm::vec3 _dir)
{
    position += _dir;
    isDirty = true;
}

void Transform::LookAt(glm::vec3 _target)
{
    glm::vec3 f = _target - position;
    if (glm::dot(f, f) < 1e-12f) return;
    f = glm::normalize(f);

    glm::vec3 worldUp = (glm::abs(f.y) > 0.999f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

    glm::vec3 r = glm::normalize(glm::cross(f, worldUp));
    glm::vec3 u = glm::cross(r, f);

    rotation = glm::quat_cast(glm::mat3(r, u, -f));
    isDirty = true;
}

void Transform::OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius)
{
}



#endif
