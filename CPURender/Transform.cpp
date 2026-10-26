#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

const glm::mat4& Transform::GetWorldMatrix() const
{
    if (m_dirty)
    {
        m_world = glm::translate(glm::mat4(1.0f), position)
                * glm::mat4_cast(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
        m_dirty = false;
    }
    return m_world;
}

void Transform::SetPosition(glm::vec3 _position)
{
    position = _position;
    MarkDirty();
}

void Transform::SetScale(glm::vec3 _scale)
{
    scale = _scale;
    MarkDirty();
}

void Transform::SetRotation(glm::quat _rotation)
{
    rotation = _rotation;
    MarkDirty();
}

void Transform::SetYawPitchRoll(glm::vec3 _euler)
{
    rotation = glm::quat(_euler);
    MarkDirty();
}

void Transform::Move(glm::vec3 _dir)
{
    position += _dir;
    MarkDirty();
}

void Transform::LookAt(glm::vec3 _target)
{
    glm::vec3 f = _target - position;
    if (glm::dot(f, f) < 1e-12f) return;
    f = glm::normalize(f);

    glm::vec3 worldUp = (glm::abs(f.y) > 0.999f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

    glm::vec3 r = glm::normalize(glm::cross(worldUp, f));
    glm::vec3 u = glm::cross(f, r);

    rotation = glm::quat_cast(glm::mat3(r, u, f));
    MarkDirty();
}

void Transform::OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius)
{
    // TODO : non implemente.
}
